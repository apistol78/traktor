#include "Core/Functor/Functor.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Log/LogStreamTarget.h"
#include "Core/Math/Const.h"
#include "Core/Misc/String.h"
#include "Core/Misc/WildCompare.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Render/Vrfy/Editor/ProgramCompilerVrfy.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/IProgramCompiler.h"
#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/FragmentLinker.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ProgramCache.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderPipeline.h"
#include "Render/Editor/Shader/ShaderGraphCombinations.h"
#include "Render/Editor/Shader/ShaderGraphHash.h"
#include "Render/Editor/Shader/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/ShaderGraphStatic.h"
#include "Render/Editor/Shader/ShaderGraphTechniques.h"
#include "Render/Editor/Shader/ShaderGraphValidator.h"
#include "Render/Resource/ProgramResource.h"
#include "Render/Resource/ShaderResource.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

#define T_USE_BUILD_COMBINATION_JOBS

namespace traktor
{
	namespace render
	{
		namespace
		{

uint32_t getPriority(const render::ShaderGraph* shaderGraph)
{
	RefArray< render::PixelOutput > nodes;
	if (shaderGraph->findNodesOf< render::PixelOutput >(nodes) == 0)
		return 0;

	uint32_t priority = 0;
	for (auto node : nodes)
	{
		if (node->getPriority() != 0)
			priority |= node->getPriority();
		else if (node->getRenderState().blendEnable)
			priority |= RpAlphaBlend;
	}

	if (priority == 0)
		priority = RpOpaque;

	return priority;
}

class FragmentReaderAdapter : public FragmentLinker::IFragmentReader
{
public:
	FragmentReaderAdapter(editor::IPipelineCommon* pipeline)
	:	m_pipeline(pipeline)
	{
	}

	virtual Ref< const ShaderGraph > read(const Guid& fragmentGuid) const
	{
		Ref< const ShaderGraph > shaderGraph = m_pipeline->getObjectReadOnly< ShaderGraph >(fragmentGuid);
		if (!shaderGraph)
			return nullptr;

		if (ShaderGraphValidator(shaderGraph, fragmentGuid).validateIntegrity())
			return shaderGraph;
		else
			return nullptr;
	}

private:
	Ref< editor::IPipelineCommon > m_pipeline;
};

struct BuildCombinationTask : public Object
{
	std::wstring name;
	std::wstring path;
	ShaderGraphCombinations* combinations;
	uint32_t combination;
	ShaderResource* shaderResource;
	ShaderResource::Technique* shaderResourceTechnique;
	ShaderResource::Combination* shaderResourceCombination;
	ProgramCache* programCache;
	const PropertyGroup* compilerSettings;
	render::IProgramCompiler::Stats stats;
	bool frequentUniformsAsLinear;
	int optimize;
	bool validate;
	bool result;
	StringOutputStream errorLog;

	void execute()
	{
		Ref< ILogTarget > localTarget = log::error.getLocalTarget();
		log::error.setLocalTarget(new LogStreamTarget(&errorLog));

		const auto& parameterBits = shaderResource->getParameterBits();

		// Remap parameter mask and value for this combination as shader consist of multiple techniques.
		uint32_t mask = combinations->getCombinationMask(combination);
		uint32_t value = combinations->getCombinationValue(combination);

		auto maskNames = combinations->getParameterNames(mask);
		auto valueNames = combinations->getParameterNames(value);

		for (const auto& maskName : maskNames)
			shaderResourceCombination->mask |= parameterBits.find(maskName)->second;
		for (const auto& valueName : valueNames)
			shaderResourceCombination->value |= parameterBits.find(valueName)->second;

		// Generate combination shader graph.
		Ref< const ShaderGraph > combinationGraph = combinations->getCombinationShaderGraph(combination);
		T_ASSERT(combinationGraph);

		// Freeze type permutation.
		Ref< ShaderGraph > programGraph = ShaderGraphStatic(combinationGraph).getTypePermutation();
		if (!programGraph)
		{
			log::error << L"ShaderPipeline failed; unable to get type permutation of \"" << path << L"\"" << Endl;
			log::error.setLocalTarget(localTarget);
			return;
		}

		// Constant propagation; calculate constant branches.
		programGraph = ShaderGraphStatic(programGraph).getConstantFolded();
		if (!programGraph)
		{
			log::error << L"ShaderPipeline failed; unable to perform constant folding of \"" << path << L"\"" << Endl;
			log::error.setLocalTarget(localTarget);
			return;
		}

		// Get output state resolved.
		programGraph = ShaderGraphStatic(programGraph).getStateResolved();
		if (!programGraph)
		{
			log::error << L"ShaderPipeline failed; unable to resolve render state of \"" << path << L"\"" << Endl;
			log::error.setLocalTarget(localTarget);
			return;
		}

		// Merge identical branches.
		programGraph = ShaderGraphOptimizer(programGraph).mergeBranches();
		if (!programGraph)
		{
			log::error << L"ShaderPipeline failed; unable to merge branches of \"" << path << L"\"" << Endl;
			log::error.setLocalTarget(localTarget);
			return;
		}

		// Insert interpolation nodes at optimal locations.
		programGraph = ShaderGraphOptimizer(programGraph).insertInterpolators(frequentUniformsAsLinear);
		if (!programGraph)
		{
			log::error << L"ShaderPipeline failed; unable to optimize shader graph \"" << path << L"\"" << Endl;
			log::error.setLocalTarget(localTarget);
			return;
		}

		// Create swizzle nodes in order to improve compiler optimizing.
		programGraph = ShaderGraphStatic(programGraph).getSwizzledPermutation();
		if (!programGraph)
		{
			log::error << L"ShaderPipeline failed; unable to perform swizzle optimization of \"" << path << L"\"" << Endl;
			log::error.setLocalTarget(localTarget);
			return;
		}

		// Remove redundant swizzle patterns.
		programGraph = ShaderGraphStatic(programGraph).cleanupRedundantSwizzles();
		if (!programGraph)
		{
			log::error << L"ShaderPipeline failed; unable to cleanup redundant swizzles of \"" << path << L"\"" << Endl;
			log::error.setLocalTarget(localTarget);
			return;
		}

		// Extract uniform initial values and add to initialization block in shader resource.
		RefArray< Uniform > uniformNodes;
		programGraph->findNodesOf< Uniform >(uniformNodes);
		for (const auto uniformNode : uniformNodes)
		{
			const OutputPin* outputPin = programGraph->findSourcePin(uniformNode->getInputPin(0));
			if (!outputPin)
				continue;

			const Node* outputNode = outputPin->getNode();
			T_ASSERT(outputNode);

			if (const Scalar* scalarNode = dynamic_type_cast< const Scalar* >(outputNode))
			{
				shaderResourceCombination->initializeUniformScalar.push_back(ShaderResource::InitializeUniformScalar(uniformNode->getParameterName(), scalarNode->get()));
			}
			else if (const Vector* vectorNode = dynamic_type_cast< const Vector* >(outputNode))
			{
				shaderResourceCombination->initializeUniformVector.push_back(ShaderResource::InitializeUniformVector(uniformNode->getParameterName(), vectorNode->get()));
			}
			else if (const Color* colorNode = dynamic_type_cast< const Color* >(outputNode))
			{
				shaderResourceCombination->initializeUniformVector.push_back(ShaderResource::InitializeUniformVector(uniformNode->getParameterName(), colorNode->getColor()));
			}
			else
			{
				log::error << L"ShaderPipeline failed; initial value of uniform must be constant" << Endl;
				log::error.setLocalTarget(localTarget);
				return;
			}
		}

		// Replace texture nodes with uniforms; keep list of texture references in shader resource.
		RefArray< Texture > textureNodes;
		programGraph->findNodesOf< Texture >(textureNodes);
		for (const auto textureNode : textureNodes)
		{
			const Guid& textureGuid = textureNode->getExternal();
			int32_t textureIndex;

			auto it = std::find(shaderResourceCombination->textures.begin(), shaderResourceCombination->textures.end(), textureGuid);
			if (it != shaderResourceCombination->textures.end())
				textureIndex = int32_t(std::distance(shaderResourceCombination->textures.begin(), it));
			else
			{
				textureIndex = int32_t(shaderResourceCombination->textures.size());
				shaderResourceCombination->textures.push_back(textureGuid);
			}

			Ref< Uniform > textureUniform = new Uniform(
				getParameterNameFromTextureReferenceIndex(textureIndex),
				textureNode->getParameterType(),
				UfOnce
			);

			const OutputPin* textureUniformOutput = textureUniform->getOutputPin(0);
			T_ASSERT(textureUniformOutput);

			const OutputPin* textureNodeOutput = textureNode->getOutputPin(0);
			T_ASSERT(textureNodeOutput);

			programGraph->rewire(textureNodeOutput, textureUniformOutput);
			programGraph->addNode(textureUniform);
		}

		// Compile shader program.
		Ref< ProgramResource > programResource = programCache->get(
			programGraph,
			compilerSettings
			// path,
			// optimize,
			// validate,
			// &stats
		);
		if (!programResource)
		{
			log::error << L"ShaderPipeline failed; unable to compile shader \"" << path << L"\"" << Endl;
			log::error.setLocalTarget(localTarget);
			return;
		}

		// Add meta tag to indicate if shader combination is opaque.
		shaderResourceCombination->priority = getPriority(programGraph);

		shaderResourceCombination->program = programResource;
		result = true;

		log::error.setLocalTarget(localTarget);
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ShaderPipeline", 88, ShaderPipeline, editor::IPipeline)

ShaderPipeline::ShaderPipeline()
:	m_frequentUniformsAsLinear(false)
,	m_optimize(4)
,	m_validate(true)
,	m_debugCompleteGraphs(false)
,	m_editor(false)
{
}

bool ShaderPipeline::create(const editor::IPipelineSettings* settings)
{
	m_programCompilerTypeName = settings->getProperty< std::wstring >(L"ShaderPipeline.ProgramCompiler");
	m_programCachePath = settings->getProperty< std::wstring >(L"ShaderPipeline.ProgramCachePath");
	m_compilerSettings = settings->getProperty< PropertyGroup >(L"ShaderPipeline.ProgramCompilerSettings");
	m_platform = settings->getProperty< std::wstring >(L"ShaderPipeline.Platform", L"");
	m_includeOnlyTechniques = settings->getProperty< std::set< std::wstring > >(L"ShaderPipeline.IncludeOnlyTechniques");
	m_frequentUniformsAsLinear = settings->getProperty< bool >(L"ShaderPipeline.FrequentUniformsAsLinear", m_frequentUniformsAsLinear);
	m_optimize = settings->getProperty< int32_t >(L"ShaderPipeline.Optimize", m_optimize);
	m_validate = settings->getProperty< bool >(L"ShaderPipeline.Validate", m_validate);
	m_debugCompleteGraphs = settings->getProperty< bool >(L"ShaderPipeline.DebugCompleteGraphs", false);
	m_debugPath = settings->getProperty< std::wstring >(L"ShaderPipeline.DebugPath", L"");
	m_editor = settings->getProperty< bool >(L"Pipeline.TargetEditor", false);
	return true;
}

void ShaderPipeline::destroy()
{
	m_programHints = nullptr;
	m_programCompiler = nullptr;
	m_programCache = nullptr;
}

TypeInfoSet ShaderPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< ShaderGraph >();
}

uint32_t ShaderPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	Ref< const ShaderGraph > shaderGraph = mandatory_non_null_type_cast< const ShaderGraph* >(sourceAsset);
	Ref< IProgramCompiler > programCompiler = getProgramCompiler();
	if (!programCompiler)
		return 0;

	std::wstring rendererSignature = programCompiler->getRendererSignature();

	if ((shaderGraph = ShaderGraphStatic(shaderGraph).getPlatformPermutation(m_platform)) == nullptr)
		return 0;

	if ((shaderGraph = ShaderGraphStatic(shaderGraph).getRendererPermutation(rendererSignature)) == nullptr)
		return 0;

	return ShaderGraphHash::calculate(shaderGraph);
}

bool ShaderPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	Ref< const ShaderGraph > shaderGraph = mandatory_non_null_type_cast< const ShaderGraph* >(sourceAsset);
	Ref< IProgramCompiler > programCompiler = getProgramCompiler();
	if (!programCompiler)
		return false;

	std::wstring rendererSignature = programCompiler->getRendererSignature();

	// Extract platform permutation.
	shaderGraph = ShaderGraphStatic(shaderGraph).getPlatformPermutation(m_platform);
	if (!shaderGraph)
	{
		log::error << L"ShaderPipeline failed; unable to get platform \"" << m_platform << L"\" permutation." << Endl;
		return false;
	}

	// Extract renderer permutation.
	shaderGraph = ShaderGraphStatic(shaderGraph).getRendererPermutation(rendererSignature);
	if (!shaderGraph)
	{
		log::error << L"ShaderPipeline failed; unable to get renderer \"" << rendererSignature << L"\" permutation." << Endl;
		return false;
	}

	// Remove unused branches; don't want to add dependencies to lingering textures et al.
	shaderGraph = ShaderGraphOptimizer(shaderGraph).removeUnusedBranches();
	T_ASSERT(shaderGraph);

	// Add fragment dependencies.
	RefArray< External > externalNodes;
	shaderGraph->findNodesOf< External >(externalNodes);

	for (auto externalNode : externalNodes)
	{
		const Guid& fragmentGuid = externalNode->getFragmentGuid();
		pipelineDepends->addDependency(fragmentGuid, editor::PdfUse);
	}

	// Add external texture dependencies.
	RefArray< Texture > textureNodes;
	shaderGraph->findNodesOf< Texture >(textureNodes);

	for (auto textureNode : textureNodes)
	{
		const Guid& textureGuid = textureNode->getExternal();
		if (textureGuid.isNotNull())
			pipelineDepends->addDependency(textureGuid, editor::PdfBuild | editor::PdfResource);
	}

	return true;
}

bool ShaderPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::PipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	Ref< const ShaderGraph > shaderGraph = mandatory_non_null_type_cast< const ShaderGraph* >(sourceAsset);

	// Get program compiler and cache; lazily created in case no program needs to be compiled.
	Ref< IProgramCompiler > programCompiler = getProgramCompiler();
	if (!programCompiler)
		return false;

	Ref< ProgramCache > programCache = getProgramCache();
	T_ASSERT(programCache);

	std::wstring rendererSignature = programCompiler->getRendererSignature();

	Ref< ShaderResource > shaderResource = new ShaderResource();
	uint32_t parameterBit = 1;

	// Resolve all local variables.
	shaderGraph = ShaderGraphStatic(shaderGraph).getVariableResolved(ShaderGraphStatic::VrtLocal);
	if (!shaderGraph)
	{
		log::error << L"ShaderPipeline failed; unable to resolve local variables." << Endl;
		return false;
	}

	// Link shader fragments.
	FragmentReaderAdapter fragmentReader(pipelineBuilder);
	shaderGraph = FragmentLinker(fragmentReader).resolve(shaderGraph, true);
	if (!shaderGraph)
	{
		log::error << L"ShaderPipeline failed; unable to link shader fragments." << Endl;
		return false;
	}

	// Resolve all global variables.
	shaderGraph = ShaderGraphStatic(shaderGraph).getVariableResolved(ShaderGraphStatic::VrtGlobal);
	if (!shaderGraph)
	{
		log::error << L"ShaderPipeline failed; unable to resolve global variables." << Endl;
		return false;
	}

	// Get connected permutation.
	shaderGraph = render::ShaderGraphStatic(shaderGraph).getConnectedPermutation();
	if (!shaderGraph)
	{
		log::error << L"ShaderPipeline failed; unable to resolve connected permutation." << Endl;
		return false;
	}

	// Extract platform permutation.
	shaderGraph = ShaderGraphStatic(shaderGraph).getPlatformPermutation(m_platform);
	if (!shaderGraph)
	{
		log::error << L"ShaderPipeline failed; unable to get platform \"" << m_platform << L"\" permutation." << Endl;
		return false;
	}

	// Extract renderer permutation.
	shaderGraph = ShaderGraphStatic(shaderGraph).getRendererPermutation(rendererSignature);
	if (!shaderGraph)
	{
		log::error << L"ShaderPipeline failed; unable to get renderer \"" << rendererSignature << L"\" permutation." << Endl;
		return false;
	}

	// Remove unused branches from shader graph.
	shaderGraph = ShaderGraphOptimizer(shaderGraph).removeUnusedBranches();
	if (!shaderGraph)
	{
		log::error << L"ShaderPipeline failed; unable to remove unused branches." << Endl;
		return false;
	}

	RefArray< ShaderGraphCombinations > shaderGraphCombinations;
	std::vector< ShaderResource::Technique* > shaderResourceTechniques;
	RefArray< BuildCombinationTask > tasks;
#if defined(T_USE_BUILD_COMBINATION_JOBS)
	RefArray< Job > jobs;
#endif

	// Generate shader graphs from techniques and combinations.
	ShaderGraphTechniques techniques(shaderGraph);

	std::set< std::wstring > techniqueNames = techniques.getNames();
	if (!m_includeOnlyTechniques.empty())
	{
		std::set< std::wstring > keepTechniqueNames;
		for (const auto& includeOnlyTechnique : m_includeOnlyTechniques)
		{
			WildCompare wc(includeOnlyTechnique);
			for (const auto& techniqueName : techniqueNames)
			{
				if (wc.match(techniqueName))
					keepTechniqueNames.insert(techniqueName);
			}
		}
		techniqueNames = keepTechniqueNames;
	}

	for (const auto& techniqueName : techniqueNames)
	{
		if (ThreadManager::getInstance().getCurrentThread()->stopped())
			break;

		log::info << L"Building shader technique \"" << techniqueName << L"\"..." << Endl;
		log::info << IncreaseIndent;

		Ref< ShaderGraph > shaderGraphTechnique = techniques.generate(techniqueName);
		T_ASSERT(shaderGraphTechnique);

		Ref< ShaderGraphCombinations > combinations = new ShaderGraphCombinations(shaderGraphTechnique);
		uint32_t combinationCount = combinations->getCombinationCount();
		shaderGraphCombinations.push_back(combinations);

		ShaderResource::Technique* shaderResourceTechnique = new ShaderResource::Technique();
		shaderResourceTechnique->name = techniqueName;
		shaderResourceTechnique->mask = 0;
		shaderResourceTechniques.push_back(shaderResourceTechnique);

		// Map parameter name to unique bits; also build parameter mask for this technique.
		for (const auto& parameterName : combinations->getParameterNames())
		{
			if (shaderResource->m_parameterBits.find(parameterName) == shaderResource->m_parameterBits.end())
			{
				shaderResource->m_parameterBits.insert(std::make_pair(parameterName, parameterBit));
				parameterBit <<= 1;
			}
			shaderResourceTechnique->mask |= shaderResource->m_parameterBits[parameterName];
		}

		// Optimize and compile all combination programs.
#if defined(T_USE_BUILD_COMBINATION_JOBS)
		log::info << L"Spawning " << combinationCount << L" tasks..." << Endl;
		JobManager& jobManager = JobManager::getInstance();
#endif
		for (uint32_t combination = 0; combination < combinationCount; ++combination)
		{
			Ref< BuildCombinationTask > task = new BuildCombinationTask();
			task->name = techniqueName;
			task->path = outputPath + L" - " + techniqueName;
			task->combinations = combinations;
			task->combination = combination;
			task->shaderResource = shaderResource;
			task->shaderResourceTechnique = shaderResourceTechnique;
			task->shaderResourceCombination = new ShaderResource::Combination;
			task->shaderResourceCombination->mask = 0;
			task->shaderResourceCombination->value = 0;
			task->programCache = programCache;
			task->compilerSettings = m_compilerSettings;
			task->frequentUniformsAsLinear = m_frequentUniformsAsLinear;
			task->optimize = m_optimize;
			task->validate = m_validate;
			task->result = false;
			tasks.push_back(task);

#if defined(T_USE_BUILD_COMBINATION_JOBS)
			Ref< Job > job = jobManager.add(makeFunctor(task.ptr(), &BuildCombinationTask::execute));
			jobs.push_back(job);
#else
			task->execute();
#endif
		}

		log::info << DecreaseIndent;
	}

#if defined(T_USE_BUILD_COMBINATION_JOBS)
	log::info << L"Collecting task(s)..." << Endl;
#endif

	render::IProgramCompiler::Stats stats;
	uint32_t failed = 0;

	for (size_t i = 0; i < tasks.size(); ++i)
	{
#if defined(T_USE_BUILD_COMBINATION_JOBS)
		if (!jobs.empty())
		{
			jobs[i]->wait();
			jobs[i] = nullptr;
		}
#endif

		if (!tasks[i]->errorLog.empty())
			log::error << tasks[i]->errorLog.str() << Endl;

		if (tasks[i]->result)
		{
			ShaderResource::Technique* technique = tasks[i]->shaderResourceTechnique;
			ShaderResource::Combination* combination = tasks[i]->shaderResourceCombination;
			technique->combinations.push_back(*combination);

			stats.vertexCost += tasks[i]->stats.vertexCost;
			stats.pixelCost += tasks[i]->stats.pixelCost;
			stats.vertexSize += tasks[i]->stats.vertexSize;
			stats.pixelSize += tasks[i]->stats.pixelSize;
		}
		else
			++failed;

		delete tasks[i]->shaderResourceCombination;
		tasks[i] = nullptr;
	}

	for (auto shaderResourceTechnique : shaderResourceTechniques)
	{
		shaderResource->m_techniques.push_back(*shaderResourceTechnique);
		delete shaderResourceTechnique;
	}

	shaderResourceTechniques.resize(0);
	shaderGraphCombinations.resize(0);

	log::info << L"All task(s) collected" << Endl;

	if (ThreadManager::getInstance().getCurrentThread()->stopped())
	{
		log::info << L"ShaderPipeline aborted; pipeline cancelled." << Endl;
		return false;
	}

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

#if defined(_DEBUG)
	// Show resource information.
	{
		const auto& techniques = shaderResource->getTechniques();
		log::info << uint32_t(techniques.size()) << L" technique(s)" << Endl;

		for (size_t i = 0; i < techniques.size(); ++i)
		{
			log::info << uint32_t(i) << L": " << techniques[i].name << Endl;
			log::info << IncreaseIndent;

			const auto& combinations = techniques[i].combinations;
			log::info << uint32_t(combinations.size()) << L" combination(s)" << Endl;

			for (size_t j = 0; j < combinations.size(); ++j)
			{
				log::info << uint32_t(j) << L": " << combinations[j].mask << L"|" << combinations[j].value << Endl;
				log::info << IncreaseIndent;

				const auto& textures = combinations[j].textures;
				log::info << uint32_t(textures.size()) << L" texture(s)" << Endl;

				for (size_t k = 0; k < textures.size(); ++k)
					log::info << uint32_t(k) << L": " << textures[k].format() << Endl;

				log::info << DecreaseIndent;
			}

			log::info << DecreaseIndent;
		}

		log::info << L"Estimated costs" << Endl;
		log::info << L"  VS " << stats.vertexCost << Endl;
		log::info << L"  PS " << stats.pixelCost << Endl;
	}
#endif

	return true;
}

Ref< ISerializable > ShaderPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	T_FATAL_ERROR;
	return nullptr;
}

IProgramCompiler* ShaderPipeline::getProgramCompiler() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_programCompilerLock);

	if (m_programCompiler)
		return m_programCompiler;

	// Find compiler class type from configured name.
	const TypeInfo* programCompilerType = TypeInfo::find(m_programCompilerTypeName.c_str());
	if (!programCompilerType)
	{
		log::error << L"Shader pipeline; unable to find program compiler type \"" << m_programCompilerTypeName << L"\"." << Endl;
		return nullptr;
	}

	// Create instance of compiler class.
	m_programCompiler = dynamic_type_cast< IProgramCompiler* >(programCompilerType->createInstance());
	if (!m_programCompiler)
	{
		log::error << L"Shader pipeline; unable to instanciate program compiler \"" << m_programCompilerTypeName << L"\"." << Endl;
		return nullptr;
	}

	// In case we're building for the editor we wrap into verification compiler.
	//if (m_editor)
	//	m_programCompiler = new ProgramCompilerVrfy(m_programCompiler);

	return m_programCompiler;
}

ProgramCache* ShaderPipeline::getProgramCache() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_programCompilerLock);

	if (m_programCache)
		return m_programCache;

	IProgramCompiler* programCompiler = getProgramCompiler();
	if (!programCompiler)
		return nullptr;

	m_programCache = new ProgramCache(
		m_programCachePath,
		programCompiler
	);

	return m_programCache;
}

	}
}
