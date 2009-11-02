#include "Render/Editor/Shader/ShaderPipeline.h"
#include "Render/Editor/Shader/ShaderGraphTechniques.h"
#include "Render/Editor/Shader/ShaderGraphCombinations.h"
#include "Render/Editor/Shader/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/ShaderGraphValidator.h"
#include "Render/IRenderSystem.h"
#include "Render/ShaderGraph.h"
#include "Render/ShaderResource.h"
#include "Render/Nodes.h"
#include "Render/External.h"
#include "Render/FragmentLinker.h"
#include "Render/ProgramResource.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineSettings.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Xml/XmlSerializer.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Stream.h"
#include "Core/Thread/JobManager.h"
#include "Core/Misc/String.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class FragmentReaderAdapter : public FragmentLinker::FragmentReader
{
public:
	FragmentReaderAdapter(editor::IPipelineBuilder* pipelineBuilder)
	:	m_pipelineBuilder(pipelineBuilder)
	{
	}

	virtual const ShaderGraph* read(const Guid& fragmentGuid)
	{
		return m_pipelineBuilder->getObjectReadOnly< ShaderGraph >(fragmentGuid);
	}

private:
	Ref< editor::IPipelineBuilder > m_pipelineBuilder;
};

struct BuildCombinationTask
{
	ShaderGraphCombinations* combinations;
	uint32_t combination;
	ShaderResource* shaderResource;
	ShaderResource::Technique* shaderResourceTechnique;
	ShaderResource::Combination* shaderResourceCombination;
	render::IRenderSystem* renderSystem;
	int optimize;
	bool validate;
	bool result;

	void execute()
	{
		const std::map< std::wstring, uint32_t >& parameterBits = shaderResource->getParameterBits();

		// Map parameter value for this combination.
		std::vector< std::wstring > parameterCombination = combinations->getParameterCombination(combination);
		for (std::vector< std::wstring >::iterator j = parameterCombination.begin(); j != parameterCombination.end(); ++j)
			shaderResourceCombination->parameterValue |= parameterBits.find(*j)->second;

		// Generate combination shader graph.
		Ref< ShaderGraph > shaderGraphCombination = combinations->generate(combination);
		T_ASSERT (shaderGraphCombination);

		// Merge identical branches.
		shaderGraphCombination = ShaderGraphOptimizer(shaderGraphCombination).mergeBranches();
		if (!shaderGraphCombination)
		{
			log::error << L"ShaderPipeline failed; unable to merge branches" << Endl;
			return;
		}

		// Insert interpolation nodes at optimal locations.
		shaderGraphCombination = ShaderGraphOptimizer(shaderGraphCombination).insertInterpolators();
		if (!shaderGraphCombination)
		{
			log::error << L"ShaderPipeline failed; unable to optimize shader graph" << Endl;
			return;
		}

		if (renderSystem)
		{
			Ref< ProgramResource > programResource = renderSystem->compileProgram(shaderGraphCombination, optimize, validate);
			if (!programResource)
			{
				log::error << L"ShaderPipeline failed; unable to compile shader" << Endl;
				return;
			}

			RefArray< Sampler > samplerNodes;
			shaderGraphCombination->findNodesOf< Sampler >(samplerNodes);

			for (RefArray< Sampler >::iterator i = samplerNodes.begin(); i != samplerNodes.end(); ++i)
			{
				const Guid& textureGuid = (*i)->getExternal();
				if (!textureGuid.isNull() && textureGuid.isValid())
				{
					programResource->addTexture(
						(*i)->getParameterName(),
						textureGuid
					);
				}
			}

			shaderResourceCombination->program = programResource;
		}
		else
			shaderResourceCombination->program = shaderGraphCombination;

		result = true;
	}
};

		}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.render.ShaderPipeline", ShaderPipeline, editor::IPipeline)

ShaderPipeline::ShaderPipeline()
:	m_optimize(4)
,	m_validate(true)
,	m_debugCompleteGraphs(false)
{
}

bool ShaderPipeline::create(const editor::IPipelineSettings* settings)
{
	if (settings->getProperty< editor::PropertyBoolean >(L"ShaderPipeline.CompileShaders", false))
	{
		std::wstring renderSystemTypeName = settings->getProperty< editor::PropertyString >(L"Editor.RenderSystem");

		const Type* renderSystemType = Type::find(renderSystemTypeName);
		if (!renderSystemType)
		{
			log::error << L"Shader pipeline; unable to find render system type \"" << renderSystemTypeName << L"\"" << Endl;
			return false;
		}

		m_renderSystem = checked_type_cast< IRenderSystem* >(renderSystemType->newInstance());
		if (!m_renderSystem)
		{
			log::error << L"Shader pipeline; unable to instanciate render system" << Endl;
			return false;
		}

		if (!m_renderSystem->create())
		{
			log::error << L"Shader pipeline; unable to create render system" << Endl;
			return false;
		}
	}

	m_optimize = settings->getProperty< editor::PropertyInteger >(L"ShaderPipeline.Optimize", m_optimize);
	m_validate = settings->getProperty< editor::PropertyBoolean >(L"ShaderPipeline.Validate", m_validate);
	m_debugCompleteGraphs = settings->getProperty< editor::PropertyBoolean >(L"ShaderPipeline.DebugCompleteGraphs", false);
	m_debugPath = settings->getProperty< editor::PropertyString >(L"ShaderPipeline.DebugPath", L"");

	log::debug << L"Using optimization level " << m_optimize << (m_validate ? L" with validation" : L" without validation") << Endl;
	return true;
}

void ShaderPipeline::destroy()
{
	if (m_renderSystem)
	{
		m_renderSystem->destroy();
		m_renderSystem = 0;
	}
}

uint32_t ShaderPipeline::getVersion() const
{
	return 5;
}

TypeSet ShaderPipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< ShaderGraph >());
	return typeSet;
}

bool ShaderPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const Serializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	Ref< const ShaderGraph > shaderGraph = checked_type_cast< const ShaderGraph* >(sourceAsset);

	// Add fragments.
	RefArray< External > externalNodes;
	shaderGraph->findNodesOf< External >(externalNodes);

	for (RefArray< External >::const_iterator i = externalNodes.begin(); i != externalNodes.end(); ++i)
	{
		const Guid& fragmentGuid = (*i)->getFragmentGuid();
		pipelineDepends->addDependency(fragmentGuid, false);
	}

	// Add external textures.
	RefArray< Sampler > samplerNodes;
	shaderGraph->findNodesOf< Sampler >(samplerNodes);

	for (RefArray< Sampler >::const_iterator i = samplerNodes.begin(); i != samplerNodes.end(); ++i)
	{
		const Guid& textureGuid = (*i)->getExternal();
		if (textureGuid.isValid() && !textureGuid.isNull())
			pipelineDepends->addDependency(textureGuid, true);
	}

	return true;
}

bool ShaderPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const Serializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	Ref< const ShaderGraph > shaderGraph = checked_type_cast< const ShaderGraph* >(sourceAsset);
	Ref< ShaderResource > shaderResource = gc_new< ShaderResource >();
	uint32_t parameterBit = 1;

	// Link shader fragments.
	FragmentReaderAdapter fragmentReader(pipelineBuilder);
	shaderGraph = FragmentLinker(fragmentReader).resolve(shaderGraph, true);
	if (!shaderGraph)
	{
		log::error << L"ShaderPipeline failed; unable to link shader fragments" << Endl;
		return false;
	}

	// Remove unused branches from shader graph.
	shaderGraph = ShaderGraphOptimizer(shaderGraph).removeUnusedBranches();
	if (!shaderGraph)
	{
		log::error << L"ShaderPipeline failed; unable to remove unused branches" << Endl;
		return false;
	}

	RefArray< ShaderGraphCombinations > shaderGraphCombinations;
	std::vector< ShaderResource::Technique* > shaderResourceTechniques;
	std::vector< BuildCombinationTask* > tasks;
	RefArray< Job > jobs;

	// Generate shader graphs from techniques and combinations.
	ShaderGraphTechniques techniques(shaderGraph);
	std::set< std::wstring > techniqueNames = techniques.getNames();

	for (std::set< std::wstring >::iterator i = techniqueNames.begin(); i != techniqueNames.end(); ++i)
	{
		log::info << L"Building shader technique \"" << *i << L"\"..." << Endl;
		log::info << IncreaseIndent;

		Ref< ShaderGraph > shaderGraphTechnique = techniques.generate(*i);
		T_ASSERT (shaderGraphTechnique);

		Ref< ShaderGraphCombinations > combinations = gc_new< ShaderGraphCombinations >(shaderGraphTechnique);
		uint32_t combinationCount = combinations->getCombinationCount();
		shaderGraphCombinations.push_back(combinations);

		ShaderResource::Technique* shaderResourceTechnique = new ShaderResource::Technique();
		shaderResourceTechnique->name = *i;
		shaderResourceTechnique->parameterMask = 0;
		shaderResourceTechniques.push_back(shaderResourceTechnique);

		// Map parameter name to unique bits; also build parameter mask for this technique.
		std::vector< std::wstring > parameterNames = combinations->getParameterNames();
		for (std::vector< std::wstring >::iterator j = parameterNames.begin(); j != parameterNames.end(); ++j)
		{
			if (shaderResource->m_parameterBits.find(*j) == shaderResource->m_parameterBits.end())
			{
				shaderResource->m_parameterBits.insert(std::make_pair(*j, parameterBit));
				parameterBit <<= 1;
			}
			shaderResourceTechnique->parameterMask |= shaderResource->m_parameterBits[*j];
		}

		// Optimize and compile all combination programs.
		log::info << L"Spawning " << combinationCount << L" tasks..." << Endl;

		for (uint32_t combination = 0; combination < combinationCount; ++combination)
		{
			BuildCombinationTask* task = new BuildCombinationTask();
			task->combinations = combinations;
			task->combination = combination;
			task->shaderResource = shaderResource;
			task->shaderResourceTechnique = shaderResourceTechnique;
			task->shaderResourceCombination = new ShaderResource::Combination;
			task->shaderResourceCombination->parameterValue = 0;
			task->renderSystem = m_renderSystem;
			task->optimize = m_optimize;
			task->validate = m_validate;
			task->result = false;

			Ref< Job > job = gc_new< Job >(makeFunctor(task, &BuildCombinationTask::execute));
			JobManager::getInstance().add(*job);

			tasks.push_back(task);
			jobs.push_back(job);
		}

		log::info << DecreaseIndent;
	}

	log::info << L"Collecting task(s)..." << Endl;

	uint32_t failed = 0;
	for (size_t i = 0; i < jobs.size(); ++i)
	{
		jobs[i]->wait();

		if (tasks[i]->result)
		{
			ShaderResource::Technique* technique = tasks[i]->shaderResourceTechnique;
			ShaderResource::Combination* combination = tasks[i]->shaderResourceCombination;
			technique->combinations.push_back(*combination);
		}
		else
			++failed;

		delete tasks[i]->shaderResourceCombination;
		delete tasks[i];
	}

	for (std::vector< ShaderResource::Technique* >::iterator i = shaderResourceTechniques.begin(); i != shaderResourceTechniques.end(); ++i)
	{
		shaderResource->m_techniques.push_back(**i);
		delete *i;
	}

	shaderResourceTechniques.resize(0);
	shaderGraphCombinations.resize(0);

	log::info << L"All task(s) collected" << Endl;

	if (failed)
	{
		log::error << L"ShaderPipeline failed; " << failed << L" task(s) failed" << Endl;
		return false;
	}

	// Create output instance.
	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!outputInstance)
	{
		log::error << L"ShaderPipeline failed; unable to create output instance" << Endl;
		return false;
	}

	outputInstance->setObject(shaderResource);

	if (!outputInstance->commit())
	{
		log::error << L"ShaderPipeline failed; unable to commit output instance" << Endl;
		outputInstance->revert();
		return false;
	}

	return true;
}

	}
}
