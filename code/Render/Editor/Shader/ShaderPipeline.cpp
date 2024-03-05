/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Log/LogStreamTarget.h"
#include "Core/Math/Const.h"
#include "Core/Misc/Preprocessor.h"
#include "Core/Misc/String.h"
#include "Core/Misc/WildCompare.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/JobManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/DataAccessCache.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Editor/PipelineDependency.h"
#include "Editor/Pipeline/PipelineProfiler.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/IProgramCompiler.h"
#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/FragmentLinker.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/Script.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderModule.h"
#include "Render/Editor/Shader/ShaderPipeline.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphCombinations.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphHash.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphStatic.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphTechniques.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphValidator.h"
#include "Render/Resource/ProgramResource.h"
#include "Render/Resource/ShaderResource.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

namespace traktor::render
{
	namespace
	{

class FragmentReaderAdapter : public FragmentLinker::IFragmentReader
{
public:
	explicit FragmentReaderAdapter(editor::IPipelineCommon* pipeline)
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

uint32_t getPriority(const render::ShaderGraph* shaderGraph)
{
	RefArray< render::PixelOutput > nodes = shaderGraph->findNodesOf< render::PixelOutput >();
	if (nodes.empty())
		return 0;

	uint32_t priority = 0;
	for (auto node : nodes)
	{
		if (node->getPriority() != 0)
			priority |= node->getPriority();
		else if (node->getRenderState().blendEnable)
			priority |= RenderPriority::AlphaBlend;
	}

	if (priority == 0)
		priority = RenderPriority::Opaque;

	return priority;
}

std::wstring resolveShaderModule(editor::IPipelineCommon* pipelineCommon, const Preprocessor& preprocessor, const Guid& id, std::set< Guid >& inoutVisited)
{
	if (!inoutVisited.insert(id).second)
		return L"";

	Ref< const ShaderModule> shaderModule = pipelineCommon->getObjectReadOnly< ShaderModule >(id);
	if (!shaderModule)
		return L"";

	const std::wstring unprocessedText = shaderModule->escape([&](const Guid& g) -> std::wstring {
		return g.format();
	});

	// Execute preprocessor on shader module.
	std::wstring text;
	std::set< std::wstring > usings;
	if (!preprocessor.evaluate(unprocessedText, text, usings))
	{
		log::error << L"Shader pipeline failed; unable to preprocess module " << id.format() << L"." << Endl;
		return L"";
	}

	// Append all usings.
	StringOutputStream ss;
	for (const auto& u : usings)
		ss << resolveShaderModule(pipelineCommon, preprocessor, Guid(u), inoutVisited);
	
	// Append module text.
	ss << text;
	return ss.str();
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ShaderPipeline", 101, ShaderPipeline, editor::IPipeline)

ShaderPipeline::ShaderPipeline()
:	m_optimize(4)
,	m_validate(true)
,	m_debugCompleteGraphs(false)
,	m_editor(false)
{
}

bool ShaderPipeline::create(const editor::IPipelineSettings* settings)
{
	m_programCompilerTypeName = settings->getPropertyIncludeHash< std::wstring >(L"ShaderPipeline.ProgramCompiler");
	m_compilerSettings = settings->getPropertyIncludeHash< PropertyGroup >(L"ShaderPipeline.ProgramCompilerSettings");
	m_platform = settings->getPropertyIncludeHash< std::wstring >(L"ShaderPipeline.Platform", L"");
	m_includeOnlyTechniques = settings->getPropertyIncludeHash< SmallSet< std::wstring > >(L"ShaderPipeline.IncludeOnlyTechniques");
	m_optimize = settings->getPropertyIncludeHash< int32_t >(L"ShaderPipeline.Optimize", m_optimize);
	m_validate = settings->getPropertyExcludeHash< bool >(L"ShaderPipeline.Validate", m_validate);
	m_debugCompleteGraphs = settings->getPropertyExcludeHash< bool >(L"ShaderPipeline.DebugCompleteGraphs", false);
	m_debugPath = settings->getPropertyExcludeHash< std::wstring >(L"ShaderPipeline.DebugPath", L"");
	m_editor = settings->getPropertyIncludeHash< bool >(L"Pipeline.TargetEditor", false);
	return true;
}

void ShaderPipeline::destroy()
{
	m_programHints = nullptr;
	m_programCompiler = nullptr;
}

TypeInfoSet ShaderPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< ShaderGraph >();
}

bool ShaderPipeline::shouldCache() const
{
	return true;
}

uint32_t ShaderPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	Ref< const ShaderGraph > shaderGraph = mandatory_non_null_type_cast< const ShaderGraph* >(sourceAsset);
	Ref< IProgramCompiler > programCompiler = getProgramCompiler();
	if (!programCompiler)
		return 0;

	const std::wstring rendererSignature = programCompiler->getRendererSignature();

	if ((shaderGraph = ShaderGraphStatic(shaderGraph, Guid()).getPlatformPermutation(m_platform)) == nullptr)
		return 0;

	if ((shaderGraph = ShaderGraphStatic(shaderGraph, Guid()).getRendererPermutation(rendererSignature)) == nullptr)
		return 0;

	return ShaderGraphHash(true, true).calculate(shaderGraph);
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

	const Guid shaderGraphId = sourceInstance ? sourceInstance->getGuid() : Guid();
	const std::wstring rendererSignature = programCompiler->getRendererSignature();

	// Extract platform permutation.
	shaderGraph = ShaderGraphStatic(shaderGraph, shaderGraphId).getPlatformPermutation(m_platform);
	if (!shaderGraph)
	{
		log::error << L"ShaderPipeline failed; unable to get platform \"" << m_platform << L"\" permutation." << Endl;
		return false;
	}

	// Extract renderer permutation.
	shaderGraph = ShaderGraphStatic(shaderGraph, shaderGraphId).getRendererPermutation(rendererSignature);
	if (!shaderGraph)
	{
		log::error << L"ShaderPipeline failed; unable to get renderer \"" << rendererSignature << L"\" permutation." << Endl;
		return false;
	}

	// Remove unused branches; don't want to add dependencies to lingering textures et al.
	shaderGraph = ShaderGraphOptimizer(shaderGraph).removeUnusedBranches(true);
	T_ASSERT(shaderGraph);

	// Add fragment, texture and text dependencies.
	for (auto node : shaderGraph->getNodes())
	{
		if (const auto externalNode = dynamic_type_cast< External* >(node))
		{
			const Guid& fragmentGuid = externalNode->getFragmentGuid();
			pipelineDepends->addDependency(fragmentGuid, editor::PdfUse);
		}
		else if (const auto textureNode = dynamic_type_cast< Texture* >(node))
		{
			const Guid& textureGuid = textureNode->getExternal();
			if (textureGuid.isNotNull())
				pipelineDepends->addDependency(textureGuid, editor::PdfBuild | editor::PdfResource);
		}
		else if (const auto scriptNode = dynamic_type_cast< Script* >(node))
		{
			for (auto includeId : scriptNode->getIncludes())
				pipelineDepends->addDependency(includeId, editor::PdfUse);
		}
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

	const Guid shaderGraphId = sourceInstance ? sourceInstance->getGuid() : Guid();
	const std::wstring rendererSignature = programCompiler->getRendererSignature();

	Ref< ShaderResource > shaderResource = new ShaderResource();
	uint32_t parameterBit = 1;

	// Resolve all variables.
	pipelineBuilder->getProfiler()->begin(L"ShaderPipeline getVariableResolved");
	shaderGraph = ShaderGraphStatic(shaderGraph, shaderGraphId).getVariableResolved();
	pipelineBuilder->getProfiler()->end();
	if (!shaderGraph)
	{
		log::error << L"ShaderPipeline failed; unable to resolve variables." << Endl;
		return false;
	}

	// Link shader fragments.
	pipelineBuilder->getProfiler()->begin(L"ShaderPipeline link fragments");
	FragmentReaderAdapter fragmentReader(pipelineBuilder);
	shaderGraph = FragmentLinker(fragmentReader).resolve(shaderGraph, true);
	pipelineBuilder->getProfiler()->end();
	if (!shaderGraph)
	{
		log::error << L"ShaderPipeline failed; unable to link shader fragments." << Endl;
		return false;
	}

	// Keep a reference to the resolved graph; in case of failure it's good for debugging.
	Ref< const ShaderGraph > resolvedGraph = shaderGraph;

	// Resolve all bundles.
	pipelineBuilder->getProfiler()->begin(L"ShaderPipeline getBundleResolved");
	shaderGraph = render::ShaderGraphStatic(shaderGraph, shaderGraphId).getBundleResolved();
	pipelineBuilder->getProfiler()->end();
	if (!shaderGraph)
	{
		log::error << L"ShaderPipeline failed; unable to resolve bundles." << Endl;
		return false;
	}

	// Get connected permutation.
	pipelineBuilder->getProfiler()->begin(L"ShaderPipeline getConnectedPermutation");
	shaderGraph = render::ShaderGraphStatic(shaderGraph, shaderGraphId).getConnectedPermutation();
	pipelineBuilder->getProfiler()->end();
	if (!shaderGraph)
	{
		log::error << L"ShaderPipeline failed; unable to resolve connected permutation." << Endl;
		return false;
	}

	// Extract platform permutation.
	pipelineBuilder->getProfiler()->begin(L"ShaderPipeline getPlatformPermutation");
	shaderGraph = ShaderGraphStatic(shaderGraph, shaderGraphId).getPlatformPermutation(m_platform);
	pipelineBuilder->getProfiler()->end();
	if (!shaderGraph)
	{
		log::error << L"ShaderPipeline failed; unable to get platform \"" << m_platform << L"\" permutation." << Endl;
		return false;
	}

	// Extract renderer permutation.
	pipelineBuilder->getProfiler()->begin(L"ShaderPipeline getRendererPermutation");
	shaderGraph = ShaderGraphStatic(shaderGraph, shaderGraphId).getRendererPermutation(rendererSignature);
	pipelineBuilder->getProfiler()->end();
	if (!shaderGraph)
	{
		log::error << L"ShaderPipeline failed; unable to get renderer \"" << rendererSignature << L"\" permutation." << Endl;
		return false;
	}

	// Remove unused branches from shader graph.
	pipelineBuilder->getProfiler()->begin(L"ShaderPipeline removeUnusedBranches");
	shaderGraph = ShaderGraphOptimizer(shaderGraph).removeUnusedBranches(false);
	pipelineBuilder->getProfiler()->end();
	if (!shaderGraph)
	{
		log::error << L"ShaderPipeline failed; unable to remove unused branches." << Endl;
		return false;
	}

	// Generate shader graphs from techniques and combinations.
	ShaderGraphTechniques techniques(shaderGraph, shaderGraphId);
	if (!techniques.valid())
		return false;

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

	struct Error
	{
		IProgramCompiler::Error error;
		std::wstring techniqueName;
		uint32_t combination;
		Ref< const ShaderGraph > resolvedGraph;
		Ref< const ShaderGraph > combinationGraph;
		Ref< const ShaderGraph > programGraph;
	};
	std::list< Error > errors;
	Semaphore errorsLock;

	for (const auto& techniqueName : techniqueNames)
	{
		if (ThreadManager::getInstance().getCurrentThread()->stopped())
			break;

		Ref< ShaderGraph > shaderGraphTechnique = techniques.generate(techniqueName);
		T_ASSERT(shaderGraphTechnique);

		Ref< ShaderGraphCombinations > combinations = new ShaderGraphCombinations(shaderGraphTechnique, shaderGraphId);
		const uint32_t combinationCount = combinations->getCombinationCount();

		log::info << L"Building shader technique \"" << techniqueName << L"\" (" << combinationCount << L" combinations)..." << Endl;
		log::info << IncreaseIndent;

		ShaderResource::Technique shaderResourceTechnique;
		shaderResourceTechnique.name = techniqueName;
		shaderResourceTechnique.mask = 0;

		// Map parameter name to unique bits; also build parameter mask for this technique.
		for (const auto& parameterName : combinations->getParameterNames())
		{
			if (shaderResource->m_parameterBits.find(parameterName) == shaderResource->m_parameterBits.end())
			{
				shaderResource->m_parameterBits.insert(std::make_pair(parameterName, parameterBit));
				parameterBit <<= 1;
			}
			shaderResourceTechnique.mask |= shaderResource->m_parameterBits[parameterName];
		}

		// Optimize and compile all combination programs.
		const std::wstring path = outputPath + L" - " + techniqueName;
		AlignedVector< Job::task_t > jobs;
		bool status = true;

		shaderResourceTechnique.combinations.resize(combinationCount);
		for (uint32_t combination = 0; combination < combinationCount; ++combination)
		{
			jobs.push_back([&, combination](){
				const auto& parameterBits = shaderResource->getParameterBits();

				// Remap parameter mask and value for this combination as shader consist of multiple techniques.
				const uint32_t mask = combinations->getCombinationMask(combination);
				const uint32_t value = combinations->getCombinationValue(combination);

				const auto maskNames = combinations->getParameterNames(mask);
				const auto valueNames = combinations->getParameterNames(value);

				ShaderResource::Combination& shaderCombination = shaderResourceTechnique.combinations[combination];
				for (const auto& maskName : maskNames)
					shaderCombination.mask |= parameterBits.find(maskName)->second;
				for (const auto& valueName : valueNames)
					shaderCombination.value |= parameterBits.find(valueName)->second;

				// Generate combination shader graph.
				Ref< const ShaderGraph > combinationGraph = combinations->getCombinationShaderGraph(combination);
				T_ASSERT(combinationGraph);

				// Freeze type permutation.
				pipelineBuilder->getProfiler()->begin(L"ShaderPipeline getTypePermutation");
				Ref< ShaderGraph > programGraph = ShaderGraphStatic(combinationGraph, shaderGraphId).getTypePermutation();
				pipelineBuilder->getProfiler()->end();
				if (!programGraph)
				{
					log::error << L"ShaderPipeline failed; unable to get type permutation of \"" << path << L"\"." << Endl;
					status = false;
					return;
				}

				// Constant propagation; calculate constant branches.
				pipelineBuilder->getProfiler()->begin(L"ShaderPipeline getConstantFolded");
				programGraph = ShaderGraphStatic(programGraph, shaderGraphId).getConstantFolded();
				pipelineBuilder->getProfiler()->end();
				if (!programGraph)
				{
					log::error << L"ShaderPipeline failed; unable to perform constant folding of \"" << path << L"\"." << Endl;
					status = false;
					return;
				}

				// Get output state resolved.
				pipelineBuilder->getProfiler()->begin(L"ShaderPipeline getStateResolved");
				programGraph = ShaderGraphStatic(programGraph, shaderGraphId).getStateResolved();
				pipelineBuilder->getProfiler()->end();
				if (!programGraph)
				{
					log::error << L"ShaderPipeline failed; unable to resolve render state of \"" << path << L"\"." << Endl;
					status = false;
					return;
				}

				// Merge identical branches.
				pipelineBuilder->getProfiler()->begin(L"ShaderPipeline mergeBranches");
				programGraph = ShaderGraphOptimizer(programGraph).mergeBranches();
				pipelineBuilder->getProfiler()->end();
				if (!programGraph)
				{
					log::error << L"ShaderPipeline failed; unable to merge branches of \"" << path << L"\"." << Endl;
					status = false;
					return;
				}

				// Insert interpolation nodes at optimal locations.
				pipelineBuilder->getProfiler()->begin(L"ShaderPipeline insertInterpolators");
				programGraph = ShaderGraphOptimizer(programGraph).insertInterpolators();
				pipelineBuilder->getProfiler()->end();
				if (!programGraph)
				{
					log::error << L"ShaderPipeline failed; unable to optimize shader graph \"" << path << L"\"." << Endl;
					status = false;
					return;
				}

				// Create swizzle nodes in order to improve compiler optimizing.
				pipelineBuilder->getProfiler()->begin(L"ShaderPipeline getSwizzledPermutation");
				programGraph = ShaderGraphStatic(programGraph, shaderGraphId).getSwizzledPermutation();
				pipelineBuilder->getProfiler()->end();
				if (!programGraph)
				{
					log::error << L"ShaderPipeline failed; unable to perform swizzle optimization of \"" << path << L"\"." << Endl;
					status = false;
					return;
				}

				// Remove redundant swizzle patterns.
				pipelineBuilder->getProfiler()->begin(L"ShaderPipeline cleanupRedundantSwizzles");
				programGraph = ShaderGraphStatic(programGraph, shaderGraphId).cleanupRedundantSwizzles();
				pipelineBuilder->getProfiler()->end();
				if (!programGraph)
				{
					log::error << L"ShaderPipeline failed; unable to cleanup redundant swizzles of \"" << path << L"\"." << Endl;
					status = false;
					return;
				}

				// Remove unused branches.
				pipelineBuilder->getProfiler()->begin(L"ShaderPipeline removeUnusedBranches");
				programGraph = ShaderGraphOptimizer(programGraph).removeUnusedBranches(false);
				pipelineBuilder->getProfiler()->end();
				if (!programGraph)
				{
					log::error << L"ShaderPipeline failed; unable to cleanup unused branches of \"" << path << L"\"." << Endl;
					status = false;
					return;
				}

				// Extract uniform initial values and add to initialization block in shader resource.
				for (const auto uniformNode : programGraph->findNodesOf< Uniform >())
				{
					const OutputPin* outputPin = programGraph->findSourcePin(uniformNode->getInputPin(0));
					if (!outputPin)
						continue;

					const Node* outputNode = outputPin->getNode();
					T_ASSERT(outputNode);

					if (const Scalar* scalarNode = dynamic_type_cast<const Scalar*>(outputNode))
					{
						shaderCombination.initializeUniformScalar.push_back(ShaderResource::InitializeUniformScalar(uniformNode->getParameterName(), scalarNode->get()));
					}
					else if (const Vector* vectorNode = dynamic_type_cast<const Vector*>(outputNode))
					{
						shaderCombination.initializeUniformVector.push_back(ShaderResource::InitializeUniformVector(uniformNode->getParameterName(), vectorNode->get()));
					}
					else if (const Color* colorNode = dynamic_type_cast<const Color*>(outputNode))
					{
						shaderCombination.initializeUniformVector.push_back(ShaderResource::InitializeUniformVector(uniformNode->getParameterName(), colorNode->getColor()));
					}
					else
					{
						log::error << L"ShaderPipeline failed; initial value of uniform must be constant." << Endl;
						status = false;
						return;
					}
				}

				// Replace texture nodes with uniforms; keep list of texture references in shader resource.
				for (const auto textureNode : programGraph->findNodesOf< Texture >())
				{
					const Guid& textureGuid = textureNode->getExternal();
					int32_t textureIndex;

					if (!textureGuid.isNotNull())
					{
						log::error << L"ShaderPipeline failed; non valid texture reference in node " << textureNode->getId().format() << L"." << Endl;
						status = false;
						return;
					}

					const auto it = std::find(shaderCombination.textures.begin(), shaderCombination.textures.end(), textureGuid);
					if (it != shaderCombination.textures.end())
						textureIndex = (int32_t)std::distance(shaderCombination.textures.begin(), it);
					else
					{
						textureIndex = (int32_t)shaderCombination.textures.size();
						shaderCombination.textures.push_back(textureGuid);
					}

					Ref< Uniform > textureUniform = new Uniform(
						getParameterNameFromTextureReferenceIndex(textureIndex),
						textureNode->getParameterType(),
						UpdateFrequency::Once
					);

					const OutputPin* textureUniformOutput = textureUniform->getOutputPin(0);
					T_ASSERT(textureUniformOutput);

					const OutputPin* textureNodeOutput = textureNode->getOutputPin(0);
					T_ASSERT(textureNodeOutput);

					programGraph->addNode(textureUniform);
					programGraph->rewire(textureNodeOutput, textureUniformOutput);
				}

				// Compile shader program.
				Preprocessor preprocessor;
				auto includeResolver = [&](const Guid& id) -> std::wstring {
					std::set< Guid > visited;
					return resolveShaderModule(pipelineBuilder, preprocessor, id, visited);
				};

				// Calculate hash of the entire shader graph including modules so we can
				// memorize shader compilation.
				uint32_t hash = ShaderGraphHash(false, false).calculate(programGraph);
				for (auto scriptNode : programGraph->findNodesOf< Script >())
				{
					for (const auto& scriptInclude : scriptNode->getIncludes())
					{
						Ref< const ShaderModule > module = pipelineBuilder->getObjectReadOnly< ShaderModule >(scriptInclude);
						if (module)
							hash += pipelineBuilder->calculateInclusiveHash(module);
					}
				}

				Ref< ProgramResource > programResource = pipelineBuilder->getDataAccessCache()->read< ProgramResource >(
					Key(0x00000000, 0x00000000, dependency->pipelineHash, hash),
					[&]() {
						pipelineBuilder->getProfiler()->begin(type_of(programCompiler));

						std::list< IProgramCompiler::Error > jobErrors;
						Ref< ProgramResource > programResource = programCompiler->compile(
							programGraph,
							m_compilerSettings,
							path,
							includeResolver,
							jobErrors
						);

						// Merge errors into output list.
						if (!jobErrors.empty())
						{
							T_ANONYMOUS_VAR(Acquire< Semaphore >)(errorsLock);
							for (const auto& jobError : jobErrors)
								errors.push_back({ jobError, techniqueName, combination, resolvedGraph, combinationGraph, programGraph });
						}

						pipelineBuilder->getProfiler()->end();
						return programResource;
					}
				);
				if (!programResource)
				{
					log::error << L"ShaderPipeline failed; unable to compile shader \"" << path << L"\"." << Endl;
					status = false;
					return;
				}

				shaderCombination.priority = getPriority(programGraph);
				shaderCombination.program = programResource;
			});
		}

		JobManager::getInstance().fork(jobs.c_ptr(), jobs.size());

		for (const auto& error : errors)
		{
			T_ANONYMOUS_VAR(ScopeIndent)(log::info);
			log::error.setIndent(0);
			log::error << Endl;
			log::error << L"-----------------------------------------------------" << Endl;
			FormatMultipleLines(log::error, error.error.source);
			log::error << L"-----------------------------------------------------" << Endl;
			log::error << error.error.message << Endl;
			log::error << L"-----------------------------------------------------" << Endl;
		}

		if (!status)
			return false;

		shaderResource->m_techniques.push_back(shaderResourceTechnique);

		log::info << DecreaseIndent;
	}

	log::info << L"All permutation(s) built." << Endl;

	// Create output instance.
	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!outputInstance)
	{
		log::error << L"ShaderPipeline failed; unable to create output instance." << Endl;
		return false;
	}

	outputInstance->setObject(shaderResource);

	if (!outputInstance->commit())
	{
		log::error << L"ShaderPipeline failed; unable to commit output instance." << Endl;
		outputInstance->revert();
		return false;
	}

	return true;
}

Ref< ISerializable > ShaderPipeline::buildProduct(
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

	if (!m_programCompiler->create(nullptr))
	{
		log::error << L"Shader pipeline; unable to create program compiler \"" << m_programCompilerTypeName << L"\"." << Endl;
		return nullptr;
	}

	// In case we're building for the editor we wrap into verification compiler.
	if (m_editor)
	{
		const TypeInfo* programCompilerVrfyType = TypeInfo::find(L"traktor.render.ProgramCompilerVrfy");
		if (programCompilerVrfyType)
		{
			Ref< IProgramCompiler > programCompilerVrfy = mandatory_non_null_type_cast< IProgramCompiler* >(programCompilerVrfyType->createInstance());
			programCompilerVrfy->create(m_programCompiler);
			m_programCompiler = programCompilerVrfy;
		}
		else
			log::warning << L"ProgramCompilerVrfy not loaded; verification of rendering not complete." << Endl;
	}

	return m_programCompiler;
}

}
