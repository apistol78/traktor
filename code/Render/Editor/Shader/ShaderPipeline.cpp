#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/JobManager.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineReport.h"
#include "Editor/IPipelineSettings.h"
#include "Render/IProgramCompiler.h"
#include "Render/Resource/FragmentLinker.h"
#include "Render/Resource/ProgramResource.h"
#include "Render/Resource/ShaderResource.h"
#include "Render/Shader/External.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderPipeline.h"
#include "Render/Editor/Shader/ShaderGraphCombinations.h"
#include "Render/Editor/Shader/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/ShaderGraphStatic.h"
#include "Render/Editor/Shader/ShaderGraphTechniques.h"
#include "Render/Editor/Shader/ShaderGraphValidator.h"
#include "Xml/XmlSerializer.h"

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

	virtual Ref< const ShaderGraph > read(const Guid& fragmentGuid)
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
	render::IProgramCompiler* programCompiler;
	render::IProgramCompiler::Stats stats;
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
		Ref< ShaderGraph > programGraph = combinations->generate(combination);
		T_ASSERT (programGraph);

		// Extract platform permutation.
		const wchar_t* platformSignature = programCompiler->getPlatformSignature();
		T_ASSERT (platformSignature);

		programGraph = ShaderGraphStatic(programGraph).getPlatformPermutation(platformSignature);
		if (!programGraph)
		{
			log::error << L"ShaderPipeline failed; unable to get platform permutation" << Endl;
			return;
		}

		// Freeze type permutation.
		programGraph = ShaderGraphStatic(programGraph).getTypePermutation();
		if (!programGraph)
		{
			log::error << L"ShaderPipeline failed; unable to get type permutation" << Endl;
			return;
		}

		// Merge identical branches.
		programGraph = ShaderGraphOptimizer(programGraph).mergeBranches();
		if (!programGraph)
		{
			log::error << L"ShaderPipeline failed; unable to merge branches" << Endl;
			return;
		}

		// Insert interpolation nodes at optimal locations.
		programGraph = ShaderGraphOptimizer(programGraph).insertInterpolators();
		if (!programGraph)
		{
			log::error << L"ShaderPipeline failed; unable to optimize shader graph" << Endl;
			return;
		}

		// Create swizzle nodes in order to improve compiler optimizing.
		programGraph = ShaderGraphStatic(programGraph).getSwizzledPermutation();
		if (!programGraph)
		{
			log::error << L"ShaderPipeline failed; unable to perform swizzle optimization" << Endl;
			return;
		}

		// Compile shader program.
		Ref< ProgramResource > programResource = programCompiler->compile(
			programGraph,
			optimize,
			validate,
			&stats
		);
		if (!programResource)
		{
			log::error << L"ShaderPipeline failed; unable to compile shader" << Endl;
			return;
		}

		shaderResourceCombination->program = programResource;
		result = true;
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ShaderPipeline", 17, ShaderPipeline, editor::IPipeline)

ShaderPipeline::ShaderPipeline()
:	m_optimize(4)
,	m_validate(true)
,	m_debugCompleteGraphs(false)
{
}

bool ShaderPipeline::create(const editor::IPipelineSettings* settings)
{
	std::wstring programCompilerTypeName = settings->getProperty< PropertyString >(L"ShaderPipeline.ProgramCompiler");

	const TypeInfo* programCompilerType = TypeInfo::find(programCompilerTypeName);
	if (!programCompilerType)
	{
		log::error << L"Shader pipeline; unable to find program compiler type \"" << programCompilerTypeName << L"\"" << Endl;
		return false;
	}

	m_programCompiler = dynamic_type_cast< IProgramCompiler* >(programCompilerType->createInstance());
	if (!m_programCompiler)
	{
		log::error << L"Shader pipeline; unable to instanciate program compiler \"" << programCompilerTypeName << L"\"" << Endl;
		return false;
	}

	m_optimize = settings->getProperty< PropertyInteger >(L"ShaderPipeline.Optimize", m_optimize);
	m_validate = settings->getProperty< PropertyBoolean >(L"ShaderPipeline.Validate", m_validate);
	m_debugCompleteGraphs = settings->getProperty< PropertyBoolean >(L"ShaderPipeline.DebugCompleteGraphs", false);
	m_debugPath = settings->getProperty< PropertyString >(L"ShaderPipeline.DebugPath", L"");

	log::debug << L"Using optimization level " << m_optimize << (m_validate ? L" with validation" : L" without validation") << Endl;
	return true;
}

void ShaderPipeline::destroy()
{
	m_programCompiler = 0;
}

TypeInfoSet ShaderPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ShaderGraph >());
	return typeSet;
}

bool ShaderPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
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
		pipelineDepends->addDependency(fragmentGuid, editor::PdfUse);
	}

	// Add external textures.
	RefArray< Texture > textureNodes;
	shaderGraph->findNodesOf< Texture >(textureNodes);

	for (RefArray< Texture >::const_iterator i = textureNodes.begin(); i != textureNodes.end(); ++i)
	{
		const Guid& textureGuid = (*i)->getExternal();
		if (textureGuid.isValid() && !textureGuid.isNull())
			pipelineDepends->addDependency(textureGuid, editor::PdfBuild);
	}

	return true;
}

bool ShaderPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	Ref< const ShaderGraph > shaderGraph = checked_type_cast< const ShaderGraph* >(sourceAsset);
	Ref< ShaderResource > shaderResource = new ShaderResource();
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
	std::vector< Job* > jobs;

	// Generate shader graphs from techniques and combinations.
	ShaderGraphTechniques techniques(shaderGraph);
	std::set< std::wstring > techniqueNames = techniques.getNames();

	for (std::set< std::wstring >::iterator i = techniqueNames.begin(); i != techniqueNames.end(); ++i)
	{
		log::info << L"Building shader technique \"" << *i << L"\"..." << Endl;
		log::info << IncreaseIndent;

		Ref< ShaderGraph > shaderGraphTechnique = techniques.generate(*i);
		T_ASSERT (shaderGraphTechnique);

		Ref< ShaderGraphCombinations > combinations = new ShaderGraphCombinations(shaderGraphTechnique);
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
			task->programCompiler = m_programCompiler;
			task->optimize = m_optimize;
			task->validate = m_validate;
			task->result = false;

			Job* job = new Job(makeFunctor(task, &BuildCombinationTask::execute));
			JobManager::getInstance().add(*job);

			tasks.push_back(task);
			jobs.push_back(job);
		}

		log::info << DecreaseIndent;
	}

	log::info << L"Collecting task(s)..." << Endl;

	render::IProgramCompiler::Stats stats;
	uint32_t failed = 0;

	for (size_t i = 0; i < jobs.size(); ++i)
	{
		jobs[i]->wait();

		if (tasks[i]->result)
		{
			ShaderResource::Technique* technique = tasks[i]->shaderResourceTechnique;
			ShaderResource::Combination* combination = tasks[i]->shaderResourceCombination;
			technique->combinations.push_back(*combination);

			stats.vertexCost += tasks[i]->stats.vertexCost;
			stats.pixelCost += tasks[i]->stats.pixelCost;
		}
		else
			++failed;

		delete tasks[i]->shaderResourceCombination;
		delete tasks[i];
		delete jobs[i];
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

	RefArray< Texture > textureNodes;
	shaderGraph->findNodesOf< Texture >(textureNodes);

	for (RefArray< Texture >::iterator i = textureNodes.begin(); i != textureNodes.end(); ++i)
	{
		const Guid& textureGuid = (*i)->getExternal();
		if (!textureGuid.isNull() && textureGuid.isValid())
			shaderResource->addTexture(textureGuid);
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

	// Create report.
	Ref< editor::IPipelineReport > report = pipelineBuilder->createReport(L"Shader", outputGuid);
	if (report)
	{
		report->set(L"path", outputPath);
		report->set(L"vertexCost", stats.vertexCost);
		report->set(L"pixelCost", stats.pixelCost);
	}

	return true;
}

	}
}
