/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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
#include "Render/Vrfy/Editor/ProgramCompilerVrfy.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/IProgramCompiler.h"
#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/FragmentLinker.h"
#include "Render/Editor/Shader/Nodes.h"
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

#include "Core/Thread/JobManager.h"

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

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ShaderPipeline", 92, ShaderPipeline, editor::IPipeline)

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
	m_programCompilerTypeName = settings->getPropertyIncludeHash< std::wstring >(L"ShaderPipeline.ProgramCompiler");
	m_compilerSettings = settings->getPropertyIncludeHash< PropertyGroup >(L"ShaderPipeline.ProgramCompilerSettings");
	m_platform = settings->getPropertyIncludeHash< std::wstring >(L"ShaderPipeline.Platform", L"");
	m_includeOnlyTechniques = settings->getPropertyIncludeHash< SmallSet< std::wstring > >(L"ShaderPipeline.IncludeOnlyTechniques");
	m_frequentUniformsAsLinear = settings->getPropertyIncludeHash< bool >(L"ShaderPipeline.FrequentUniformsAsLinear", m_frequentUniformsAsLinear);
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

	std::wstring rendererSignature = programCompiler->getRendererSignature();

	if ((shaderGraph = ShaderGraphStatic(shaderGraph).getPlatformPermutation(m_platform)) == nullptr)
		return 0;

	if ((shaderGraph = ShaderGraphStatic(shaderGraph).getRendererPermutation(rendererSignature)) == nullptr)
		return 0;

	return ShaderGraphHash(true).calculate(shaderGraph);
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
	shaderGraph = ShaderGraphOptimizer(shaderGraph).removeUnusedBranches(true);
	T_ASSERT(shaderGraph);

	// Add fragment and texture dependencies.
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
	shaderGraph = ShaderGraphOptimizer(shaderGraph).removeUnusedBranches(false);
	if (!shaderGraph)
	{
		log::error << L"ShaderPipeline failed; unable to remove unused branches." << Endl;
		return false;
	}

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

		Ref< ShaderGraph > shaderGraphTechnique = techniques.generate(techniqueName);
		T_ASSERT(shaderGraphTechnique);

		Ref< ShaderGraphCombinations > combinations = new ShaderGraphCombinations(shaderGraphTechnique);
		uint32_t combinationCount = combinations->getCombinationCount();

		log::info << L"Building shader technique \"" << techniqueName << L"\" (" << combinationCount << L" permutations)..." << Endl;
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
				uint32_t mask = combinations->getCombinationMask(combination);
				uint32_t value = combinations->getCombinationValue(combination);

				auto maskNames = combinations->getParameterNames(mask);
				auto valueNames = combinations->getParameterNames(value);

				ShaderResource::Combination& shaderCombination = shaderResourceTechnique.combinations[combination];
				for (const auto& maskName : maskNames)
					shaderCombination.mask |= parameterBits.find(maskName)->second;
				for (const auto& valueName : valueNames)
					shaderCombination.value |= parameterBits.find(valueName)->second;

				// Generate combination shader graph.
				Ref< const ShaderGraph > combinationGraph = combinations->getCombinationShaderGraph(combination);
				T_ASSERT(combinationGraph);

				// Freeze type permutation.
				Ref< ShaderGraph > programGraph = ShaderGraphStatic(combinationGraph).getTypePermutation();
				if (!programGraph)
				{
					log::error << L"ShaderPipeline failed; unable to get type permutation of \"" << path << L"\"." << Endl;
					status = false;
					return;
				}

				// Constant propagation; calculate constant branches.
				programGraph = ShaderGraphStatic(programGraph).getConstantFolded();
				if (!programGraph)
				{
					log::error << L"ShaderPipeline failed; unable to perform constant folding of \"" << path << L"\"." << Endl;
					status = false;
					return;
				}

				// Get output state resolved.
				programGraph = ShaderGraphStatic(programGraph).getStateResolved();
				if (!programGraph)
				{
					log::error << L"ShaderPipeline failed; unable to resolve render state of \"" << path << L"\"." << Endl;
					status = false;
					return;
				}

				// Merge identical branches.
				programGraph = ShaderGraphOptimizer(programGraph).mergeBranches();
				if (!programGraph)
				{
					log::error << L"ShaderPipeline failed; unable to merge branches of \"" << path << L"\"." << Endl;
					status = false;
					return;
				}

				// Insert interpolation nodes at optimal locations.
				programGraph = ShaderGraphOptimizer(programGraph).insertInterpolators(m_frequentUniformsAsLinear);
				if (!programGraph)
				{
					log::error << L"ShaderPipeline failed; unable to optimize shader graph \"" << path << L"\"." << Endl;
					status = false;
					return;
				}

				// Create swizzle nodes in order to improve compiler optimizing.
				programGraph = ShaderGraphStatic(programGraph).getSwizzledPermutation();
				if (!programGraph)
				{
					log::error << L"ShaderPipeline failed; unable to perform swizzle optimization of \"" << path << L"\"." << Endl;
					status = false;
					return;
				}

				// Remove redundant swizzle patterns.
				programGraph = ShaderGraphStatic(programGraph).cleanupRedundantSwizzles();
				if (!programGraph)
				{
					log::error << L"ShaderPipeline failed; unable to cleanup redundant swizzles of \"" << path << L"\"." << Endl;
					status = false;
					return;
				}

				// Remove unused branches.
				programGraph = ShaderGraphOptimizer(programGraph).removeUnusedBranches(false);
				if (!programGraph)
				{
					log::error << L"ShaderPipeline failed; unable to cleanup unused branches of \"" << path << L"\"." << Endl;
					status = false;
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
				RefArray< Texture > textureNodes;
				programGraph->findNodesOf< Texture >(textureNodes);
				for (const auto textureNode : textureNodes)
				{
					const Guid& textureGuid = textureNode->getExternal();
					int32_t textureIndex;

					if (!textureGuid.isNotNull())
					{
						log::error << L"ShaderPipeline failed; non valid texture reference in node " << textureNode->getId().format() << L"." << Endl;
						status = false;
						return;
					}

					auto it = std::find(shaderCombination.textures.begin(), shaderCombination.textures.end(), textureGuid);
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

					programGraph->rewire(textureNodeOutput, textureUniformOutput);
					programGraph->addNode(textureUniform);
				}

				// Compile shader program.
				uint32_t hash = ShaderGraphHash(false).calculate(programGraph);
				Ref< ProgramResource > programResource = pipelineBuilder->getDataAccessCache()->read< ProgramResource >(
					Key(0x00000000, 0x00000000, dependency->pipelineHash, hash),
					[&](IStream* stream) {
						return BinarySerializer(stream).readObject< ProgramResource >();
					},
					[&](const ProgramResource* object, IStream* stream) {
						return BinarySerializer(stream).writeObject(object);
					},
					[&]() {
						pipelineBuilder->getProfiler()->begin(type_of(programCompiler));
						Ref< ProgramResource > programResource = programCompiler->compile(programGraph, m_compilerSettings, path, nullptr);
						pipelineBuilder->getProfiler()->end(type_of(programCompiler));
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

	// In case we're building for the editor we wrap into verification compiler.
	//if (m_editor)
	//	m_programCompiler = new ProgramCompilerVrfy(m_programCompiler);

	return m_programCompiler;
}

	}
}
