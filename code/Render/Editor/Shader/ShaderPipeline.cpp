#include "Render/Editor/Shader/ShaderPipeline.h"
#include "Render/Editor/Shader/ShaderGraphTechniques.h"
#include "Render/Editor/Shader/ShaderGraphCombinations.h"
#include "Render/Editor/Shader/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/ShaderGraphValidator.h"
#include "Render/RenderSystem.h"
#include "Render/ShaderGraph.h"
#include "Render/ShaderResource.h"
#include "Render/Nodes.h"
#include "Render/External.h"
#include "Render/FragmentLinker.h"
#include "Render/ProgramResource.h"
#include "Editor/PipelineManager.h"
#include "Editor/Settings.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Xml/XmlSerializer.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Stream.h"
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
	FragmentReaderAdapter(editor::PipelineManager* pipelineManager)
	:	m_pipelineManager(pipelineManager)
	{
	}

	virtual const ShaderGraph* read(const Guid& fragmentGuid)
	{
		return m_pipelineManager->getObjectReadOnly< ShaderGraph >(fragmentGuid);
	}

private:
	Ref< editor::PipelineManager > m_pipelineManager;
};

/*! \brief Traverse shader graphs to find all dependencies. */
void traverseDependencies(editor::PipelineManager* pipelineManager, const ShaderGraph* shaderGraph, std::set< Guid >& outDependencies)
{
	// Traverse fragment graphs.
	RefArray< External > externalNodes;
	shaderGraph->findNodesOf< External >(externalNodes);

	for (RefArray< External >::const_iterator i = externalNodes.begin(); i != externalNodes.end(); ++i)
	{
		const Guid& fragmentGuid = (*i)->getFragmentGuid();

		if (outDependencies.find(fragmentGuid) != outDependencies.end())
			continue;

		outDependencies.insert(fragmentGuid);

		Ref< const ShaderGraph > fragmentGraph = pipelineManager->getObjectReadOnly< ShaderGraph >(fragmentGuid);
		if (fragmentGraph)
		{
			pipelineManager->addDependency(fragmentGuid, false);
			traverseDependencies(pipelineManager, fragmentGraph, outDependencies);
		}
	}

	// Add external textures.
	RefArray< Sampler > samplerNodes;
	shaderGraph->findNodesOf< Sampler >(samplerNodes);

	for (RefArray< Sampler >::const_iterator i = samplerNodes.begin(); i != samplerNodes.end(); ++i)
	{
		const Guid& textureGuid = (*i)->getExternal();
		if (textureGuid.isValid() && !textureGuid.isNull())
		{
			outDependencies.insert(textureGuid);
			pipelineManager->addDependency(textureGuid);
		}
	}
}

		}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.render.ShaderPipeline", ShaderPipeline, editor::IPipeline)

ShaderPipeline::ShaderPipeline()
:	m_optimize(4)
,	m_validate(true)
,	m_debugCompleteGraphs(false)
{
}

bool ShaderPipeline::create(const editor::Settings* settings)
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

		m_renderSystem = checked_type_cast< RenderSystem* >(renderSystemType->newInstance());
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

	log::info << L"Using optimization level " << m_optimize << (m_validate ? L" with validation" : L" without validation") << Endl;
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
	return 4;
}

TypeSet ShaderPipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< ShaderGraph >());
	return typeSet;
}

bool ShaderPipeline::buildDependencies(
	editor::PipelineManager* pipelineManager,
	const Serializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	Ref< const ShaderGraph > shaderGraph = checked_type_cast< const ShaderGraph* >(sourceAsset);

	std::set< Guid > dependencies;
	traverseDependencies(pipelineManager, shaderGraph, dependencies);

	return true;
}

bool ShaderPipeline::buildOutput(
	editor::PipelineManager* pipelineManager,
	const Serializable* sourceAsset,
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
	FragmentReaderAdapter fragmentReader(pipelineManager);
	shaderGraph = FragmentLinker(fragmentReader).resolve(shaderGraph);
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

#if defined(_DEBUG)
	// Ensure shader graph is valid as a shader.
	if (!ShaderGraphValidator(shaderGraph).validate(ShaderGraphValidator::SgtShader))
	{
		log::error << L"ShaderPipeline failed; not a valid shader graph" << Endl;
		return false;
	}
#endif

	// Generate shader graphs from techniques and combinations.
	ShaderGraphTechniques techniques(shaderGraph);
	std::set< std::wstring > techniqueNames = techniques.getNames();

	for (std::set< std::wstring >::iterator i = techniqueNames.begin(); i != techniqueNames.end(); ++i)
	{
		log::info << L"Building shader technique \"" << *i << L"\"..." << Endl;
		log::info << IncreaseIndent;

		Ref< ShaderGraph > shaderGraphTechnique = techniques.generate(*i);
		T_ASSERT (shaderGraphTechnique);

#if defined(_DEBUG)
		// Ensure shader graph is valid as a program.
		if (!ShaderGraphValidator(shaderGraphTechnique).validate(ShaderGraphValidator::SgtShader))
		{
			log::error << L"ShaderPipeline failed; not a valid shader graph" << Endl;
			return false;
		}
#endif
		ShaderGraphCombinations combinations(shaderGraphTechnique);
		uint32_t combinationCount = combinations.getCombinationCount();

		ShaderResource::Technique shaderResourceTechnique;
		shaderResourceTechnique.name = *i;
		shaderResourceTechnique.parameterMask = 0;

		// Map parameter name to unique bits; also build parameter mask for this technique.
		std::vector< std::wstring > parameterNames = combinations.getParameterNames();
		for (std::vector< std::wstring >::iterator j = parameterNames.begin(); j != parameterNames.end(); ++j)
		{
			if (shaderResource->m_parameterBits.find(*j) == shaderResource->m_parameterBits.end())
			{
				shaderResource->m_parameterBits.insert(std::make_pair(*j, parameterBit));
				parameterBit <<= 1;
			}
			shaderResourceTechnique.parameterMask |= shaderResource->m_parameterBits[*j];
		}

		// Optimize and compile all combination programs.
		for (uint32_t combination = 0; combination < combinationCount; ++combination)
		{
			log::info << L"Building combination " << (combination + 1) << L"/" << combinationCount << L"..." << Endl;
			log::info << IncreaseIndent;

			ShaderResource::Combination shaderResourceCombination;
			shaderResourceCombination.parameterValue = 0;

			// Map parameter value for this combination.
			std::vector< std::wstring > parameterCombination = combinations.getParameterCombination(combination);
			for (std::vector< std::wstring >::iterator j = parameterCombination.begin(); j != parameterCombination.end(); ++j)
				shaderResourceCombination.parameterValue |= shaderResource->m_parameterBits[*j];

			// Generate combination shader graph.
			Ref< ShaderGraph > shaderGraphCombination = combinations.generate(combination);
			T_ASSERT (shaderGraphCombination);

#if defined(_DEBUG)
			// Ensure shader graph is valid as a program.
			if (!ShaderGraphValidator(shaderGraphCombination).validate(ShaderGraphValidator::SgtProgram))
			{
				log::error << L"ShaderPipeline failed; not a valid shader graph" << Endl;
				return false;
			}
#endif
			// Merge identical branches.
			log::info << L"Merging identical branches..." << Endl;
			shaderGraphCombination = ShaderGraphOptimizer(shaderGraphCombination).mergeBranches();
			if (!shaderGraphCombination)
			{
				log::error << L"ShaderPipeline failed; unable to merge branches" << Endl;
				return false;
			}

#if defined(_DEBUG)
			// Ensure shader graph is valid as a program.
			if (!ShaderGraphValidator(shaderGraphCombination).validate(ShaderGraphValidator::SgtProgram))
			{
				log::error << L"ShaderPipeline failed; not a valid shader graph" << Endl;
				return false;
			}
#endif
			// Insert interpolation nodes at optimal locations.
			log::info << L"Inserting interpolators..." << Endl;
			shaderGraphCombination = ShaderGraphOptimizer(shaderGraphCombination).insertInterpolators();
			if (!shaderGraphCombination)
			{
				log::error << L"ShaderPipeline failed; unable to optimize shader graph" << Endl;
				return false;
			}

			// Write complete shader graphs.
			if (m_debugCompleteGraphs)
			{
				Path path = m_debugPath + L"/" + outputPath + L"_" + *i + L"_" + toString(combination + 1) + L".xml";
				FileSystem::getInstance().makeAllDirectories(path.getPathOnly());

				Ref< Stream > file = FileSystem::getInstance().open(path, File::FmWrite);
				if (file)
				{
					xml::XmlSerializer(file).writeObject(shaderGraphCombination);
					file->close();
				}
				else
					log::warning << L"Unable to create debug file" << Endl;
			}

#if defined(_DEBUG)
			// Ensure shader graph is valid as a program.
			if (!ShaderGraphValidator(shaderGraphCombination).validate(ShaderGraphValidator::SgtProgram))
			{
				log::error << L"ShaderPipeline failed; not a valid shader graph" << Endl;
				return false;
			}
#endif

			if (m_renderSystem)
			{
				log::info << L"Compiling program..." << Endl;
				Ref< ProgramResource > programResource = m_renderSystem->compileProgram(shaderGraphCombination, m_optimize, m_validate);
				if (!programResource)
				{
					log::error << L"ShaderPipeline failed; unable to compile shader" << Endl;
					return false;
				}

				RefArray< Sampler > samplerNodes;
				shaderGraphCombination->findNodesOf< Sampler >(samplerNodes);

				for (RefArray< Sampler >::iterator i = samplerNodes.begin(); i != samplerNodes.end(); ++i)
				{
					const Guid& textureGuid = (*i)->getExternal();
					if (!textureGuid.isNull() && textureGuid.isValid())
					{
						programResource->m_textures.push_back(std::make_pair(
							(*i)->getParameterName(),
							textureGuid
						));
					}
				}

				shaderResourceCombination.program = programResource;
			}
			else
				shaderResourceCombination.program = shaderGraphCombination;

			shaderResourceTechnique.combinations.push_back(shaderResourceCombination);
			log::info << DecreaseIndent;
		}

		shaderResource->m_techniques.push_back(shaderResourceTechnique);
		log::info << DecreaseIndent;
	}

	// Create output instance.
	Ref< db::Instance > outputInstance = pipelineManager->createOutputInstance(
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
