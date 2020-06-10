#include <cstring>
#include <sstream>

#include <glslang/Include/ShHandle.h>
#include <glslang/Include/revision.h>
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <SPIRV/GLSL.std.450.h>
#include <SPIRV/doc.h>
#include <SPIRV/disassemble.h>

#include <spirv-tools/optimizer.hpp>

#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"

#include "Render/Vulkan/ProgramResourceVk.h"
#include "Render/Vulkan/Editor/ProgramCompilerVk.h"
#include "Render/Vulkan/Editor/Glsl/GlslContext.h"
#include "Render/Vulkan/Editor/Glsl/GlslImage.h"
#include "Render/Vulkan/Editor/Glsl/GlslSampler.h"
#include "Render/Vulkan/Editor/Glsl/GlslStorageBuffer.h"
#include "Render/Vulkan/Editor/Glsl/GlslTexture.h"
#include "Render/Vulkan/Editor/Glsl/GlslUniformBuffer.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

TBuiltInResource getDefaultBuiltInResource()
{
	TBuiltInResource bir = {};

	bir.maxLights = 32;
	bir.maxClipPlanes = 6;
	bir.maxTextureUnits = 32;
	bir.maxTextureCoords = 32;
	bir.maxVertexAttribs = 64;
	bir.maxVertexUniformComponents = 4096;
	bir.maxVaryingFloats = 64;
	bir.maxVertexTextureImageUnits = 32;
	bir.maxCombinedTextureImageUnits = 80;
	bir.maxTextureImageUnits = 32;
	bir.maxFragmentUniformComponents = 4096;
	bir.maxDrawBuffers = 32;
	bir.maxVertexUniformVectors = 128;
	bir.maxVaryingVectors = 8;
	bir.maxFragmentUniformVectors = 16;
	bir.maxVertexOutputVectors = 16;
	bir.maxFragmentInputVectors = 15;
	bir.minProgramTexelOffset = -8;
	bir.maxProgramTexelOffset = 7;
	bir.maxClipDistances = 8;
	bir.maxComputeWorkGroupCountX = 65535;
	bir.maxComputeWorkGroupCountY = 65535;
	bir.maxComputeWorkGroupCountZ = 65535;
	bir.maxComputeWorkGroupSizeX = 1024;
	bir.maxComputeWorkGroupSizeY = 1024;
	bir.maxComputeWorkGroupSizeZ = 64;
	bir.maxComputeUniformComponents = 1024;
	bir.maxComputeTextureImageUnits = 16;
	bir.maxComputeImageUniforms = 8;
	bir.maxComputeAtomicCounters = 8;
	bir.maxComputeAtomicCounterBuffers = 1;
	bir.maxVaryingComponents = 60;
	bir.maxVertexOutputComponents = 64;
	bir.maxGeometryInputComponents = 64;
	bir.maxGeometryOutputComponents = 128;
	bir.maxFragmentInputComponents = 128;
	bir.maxImageUnits = 8;
	bir.maxCombinedImageUnitsAndFragmentOutputs = 8;
	bir.maxCombinedShaderOutputResources = 8;
	bir.maxImageSamples = 0;
	bir.maxVertexImageUniforms = 0;
	bir.maxTessControlImageUniforms = 0;
	bir.maxTessEvaluationImageUniforms = 0;
	bir.maxGeometryImageUniforms = 0;
	bir.maxFragmentImageUniforms = 8;
	bir.maxCombinedImageUniforms = 8;
	bir.maxGeometryTextureImageUnits = 16;
	bir.maxGeometryOutputVertices = 256;
	bir.maxGeometryTotalOutputComponents = 1024;
	bir.maxGeometryUniformComponents = 1024;
	bir.maxGeometryVaryingComponents = 64;
	bir.maxTessControlInputComponents = 128;
	bir.maxTessControlOutputComponents = 128;
	bir.maxTessControlTextureImageUnits = 16;
	bir.maxTessControlUniformComponents = 1024;
	bir.maxTessControlTotalOutputComponents = 4096;
	bir.maxTessEvaluationInputComponents = 128;
	bir.maxTessEvaluationOutputComponents = 128;
	bir.maxTessEvaluationTextureImageUnits = 16;
	bir.maxTessEvaluationUniformComponents = 1024;
	bir.maxTessPatchComponents = 120;
	bir.maxPatchVertices = 32;
	bir.maxTessGenLevel = 64;
	bir.maxViewports = 16;
	bir.maxVertexAtomicCounters = 0;
	bir.maxTessControlAtomicCounters = 0;
	bir.maxTessEvaluationAtomicCounters = 0;
	bir.maxGeometryAtomicCounters = 0;
	bir.maxFragmentAtomicCounters = 8;
	bir.maxCombinedAtomicCounters = 8;
	bir.maxAtomicCounterBindings = 1;
	bir.maxVertexAtomicCounterBuffers = 0;
	bir.maxTessControlAtomicCounterBuffers = 0;
	bir.maxTessEvaluationAtomicCounterBuffers = 0;
	bir.maxGeometryAtomicCounterBuffers = 0;
	bir.maxFragmentAtomicCounterBuffers = 1;
	bir.maxCombinedAtomicCounterBuffers = 1;
	bir.maxAtomicCounterBufferSize = 16384;
	bir.maxTransformFeedbackBuffers = 4;
	bir.maxTransformFeedbackInterleavedComponents = 64;
	bir.maxCullDistances = 8;
	bir.maxCombinedClipAndCullDistances = 8;
	bir.maxSamples = 4;

	bir.maxMeshOutputVerticesNV = 0;
    bir.maxMeshOutputPrimitivesNV = 0;
    bir.maxMeshWorkGroupSizeX_NV = 0;
    bir.maxMeshWorkGroupSizeY_NV = 0;
    bir.maxMeshWorkGroupSizeZ_NV = 0;
    bir.maxTaskWorkGroupSizeX_NV = 0;
    bir.maxTaskWorkGroupSizeY_NV = 0;
    bir.maxTaskWorkGroupSizeZ_NV = 0;
    bir.maxMeshViewCountNV = 0;
	bir.maxDualSourceDrawBuffersEXT = 0;

	bir.limits.nonInductiveForLoops = 1;
	bir.limits.whileLoops = 1;
	bir.limits.doWhileLoops = 1;
	bir.limits.generalUniformIndexing = 1;
	bir.limits.generalAttributeMatrixVectorIndexing = 1;
	bir.limits.generalVaryingIndexing = 1;
	bir.limits.generalSamplerIndexing = 1;
	bir.limits.generalVariableIndexing = 1;
	bir.limits.generalConstantMatrixVectorIndexing = 1;

	return bir;
}

const uint32_t c_parameterTypeWidths[] = { 1, 4, 16, 0, 0, 0 };

void performOptimization(AlignedVector< uint32_t >& spirv)
{
	spv_target_env target_env = SPV_ENV_UNIVERSAL_1_2;

	spvtools::Optimizer optimizer(target_env);
	optimizer.SetMessageConsumer([](spv_message_level_t level, const char* source, const spv_position_t& position, const char* message) {
		switch (level)
		{
		case SPV_MSG_FATAL:
		case SPV_MSG_INTERNAL_ERROR:
		case SPV_MSG_ERROR:
			log::error << L"SPIRV optimization error: ";
			if (source)
				log::error << mbstows(source) << L":";
			log::error << position.line << L":" << position.column << L":" << position.index << L":";
			if (message)
				log::error << L" " << mbstows(message);
			log::error << Endl;
			break;

		case SPV_MSG_WARNING:
			log::warning << L"SPIRV optimization warning: ";
			if (source)
				log::warning << mbstows(source) << L":";
			log::warning << position.line << L":" << position.column << L":" << position.index << L":";
			if (message)
				log::warning << L" " << mbstows(message);
			log::warning << Endl;
			break;

		case SPV_MSG_INFO:
		case SPV_MSG_DEBUG:
				log::info << L"SPIRV optimization info: ";
			if (source)
				log::info << mbstows(source) << L":";
			log::info << position.line << L":" << position.column << L":" << position.index << L":";
			if (message)
				log::info << L" " << mbstows(message);
			log::info << Endl;
			break;

		default:
			break;
		}
	});

	// If debug (specifically source line info) is being generated, propagate
	// line information into all SPIR-V instructions. This avoids loss of
	// information when instructions are deleted or moved. Later, remove
	// redundant information to minimize final SPRIR-V size.
	//if (options->generateDebugInfo) {
	//	optimizer.RegisterPass(spvtools::CreatePropagateLineInfoPass());
	//}
	optimizer.RegisterPass(spvtools::CreateWrapOpKillPass());
	optimizer.RegisterPass(spvtools::CreateDeadBranchElimPass());
	optimizer.RegisterPass(spvtools::CreateMergeReturnPass());
	optimizer.RegisterPass(spvtools::CreateInlineExhaustivePass());
	optimizer.RegisterPass(spvtools::CreateEliminateDeadFunctionsPass());
	optimizer.RegisterPass(spvtools::CreateScalarReplacementPass());
	optimizer.RegisterPass(spvtools::CreateLocalAccessChainConvertPass());
	optimizer.RegisterPass(spvtools::CreateLocalSingleBlockLoadStoreElimPass());
	optimizer.RegisterPass(spvtools::CreateLocalSingleStoreElimPass());
	optimizer.RegisterPass(spvtools::CreateSimplificationPass());
	optimizer.RegisterPass(spvtools::CreateAggressiveDCEPass());
	optimizer.RegisterPass(spvtools::CreateVectorDCEPass());
	optimizer.RegisterPass(spvtools::CreateDeadInsertElimPass());
	optimizer.RegisterPass(spvtools::CreateAggressiveDCEPass());
	optimizer.RegisterPass(spvtools::CreateDeadBranchElimPass());
	optimizer.RegisterPass(spvtools::CreateBlockMergePass());
	optimizer.RegisterPass(spvtools::CreateLocalMultiStoreElimPass());
	optimizer.RegisterPass(spvtools::CreateIfConversionPass());
	optimizer.RegisterPass(spvtools::CreateSimplificationPass());
	optimizer.RegisterPass(spvtools::CreateAggressiveDCEPass());
	optimizer.RegisterPass(spvtools::CreateVectorDCEPass());
	optimizer.RegisterPass(spvtools::CreateDeadInsertElimPass());
	//if (options->optimizeSize) {
	//	optimizer.RegisterPass(spvtools::CreateRedundancyEliminationPass());
	//}
	optimizer.RegisterPass(spvtools::CreateAggressiveDCEPass());
	optimizer.RegisterPass(spvtools::CreateCFGCleanupPass());
	//if (options->generateDebugInfo) {
	//	optimizer.RegisterPass(spvtools::CreateRedundantLineInfoElimPass());
	//}

	spvtools::OptimizerOptions spvOptOptions;
	//optimizer.SetTargetEnv(MapToSpirvToolsEnv(intermediate.getSpv(), logger));
	//spvOptOptions.set_run_validator(false); // The validator may run as a separate step later on

	std::vector< uint32_t > opted;
	if (optimizer.Run(spirv.c_ptr(), spirv.size(), &opted, spvOptOptions))
		spirv = AlignedVector< uint32_t >(opted.begin(), opted.end());
	else
		log::warning << L"SPIR-V optimizer failed; using unoptimized IL." << Endl;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerVk", 0, ProgramCompilerVk, IProgramCompiler)

ProgramCompilerVk::ProgramCompilerVk()
{
	static bool s_initialized = false;
	if (!s_initialized)
	{
		glslang::InitializeProcess();
		s_initialized = true;
	}
}

const wchar_t* ProgramCompilerVk::getRendererSignature() const
{
	return L"Vulkan";
}

Ref< ProgramResource > ProgramCompilerVk::compile(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	const std::wstring& name,
	int32_t optimize,
	bool validate,
	Stats* outStats
) const
{
	RefArray< VertexOutput > vertexOutputs;
	RefArray< PixelOutput > pixelOutputs;
	RefArray< ComputeOutput > computeOutputs;

	shaderGraph->findNodesOf< VertexOutput >(vertexOutputs);
	shaderGraph->findNodesOf< PixelOutput >(pixelOutputs);
	shaderGraph->findNodesOf< ComputeOutput >(computeOutputs);

	GlslContext cx(shaderGraph);

	glslang::TProgram* program = new glslang::TProgram();
	glslang::TShader* vertexShader = nullptr;
	glslang::TShader* fragmentShader = nullptr;
	glslang::TShader* computeShader = nullptr;

	if (vertexOutputs.size() == 1 && pixelOutputs.size() == 1)
	{
		const auto defaultBuiltInResource = getDefaultBuiltInResource();

		cx.getEmitter().emit(cx, pixelOutputs[0]);
		cx.getEmitter().emit(cx, vertexOutputs[0]);

		GlslRequirements vertexRequirements = cx.requirements();
		vertexRequirements.precisionHint = PhHigh;

		GlslRequirements fragmentRequirements = cx.requirements();

		const auto& layout = cx.getLayout();
		const char* vertexShaderText = strdup(wstombs(cx.getVertexShader().getGeneratedShader(layout, vertexRequirements)).c_str());
		const char* fragmentShaderText = strdup(wstombs(cx.getFragmentShader().getGeneratedShader(layout, fragmentRequirements)).c_str());

		// Vertex shader.
		vertexShader = new glslang::TShader(EShLangVertex);
		vertexShader->setStrings(&vertexShaderText, 1);
		vertexShader->setEntryPoint("main");

		bool vertexResult = vertexShader->parse(&defaultBuiltInResource, 100, false, (EShMessages)(EShMsgVulkanRules | EShMsgSpvRules));

		if (vertexShader->getInfoLog())
		{
			std::wstring info = trim(mbstows(vertexShader->getInfoLog()));
			if (!info.empty())
				log::info << info << Endl;
		}

#if defined(_DEBUG)
		if (vertexShader->getInfoDebugLog())
			log::info << mbstows(vertexShader->getInfoDebugLog()) << Endl;
#endif

		if (!vertexResult)
			return nullptr;

		program->addShader(vertexShader);

		// Fragment shader.
		fragmentShader = new glslang::TShader(EShLangFragment);
		fragmentShader->setStrings(&fragmentShaderText, 1);
		fragmentShader->setEntryPoint("main");

		bool fragmentResult = fragmentShader->parse(&defaultBuiltInResource, 100, false, (EShMessages)(EShMsgVulkanRules | EShMsgSpvRules));

		if (fragmentShader->getInfoLog())
		{
			std::wstring info = trim(mbstows(fragmentShader->getInfoLog()));
			if (!info.empty())
				log::info << info << Endl;
		}

#if defined(_DEBUG)
		if (fragmentShader->getInfoDebugLog())
			log::info << mbstows(fragmentShader->getInfoDebugLog()) << Endl;
#endif

		if (!fragmentResult)
			return nullptr;

		program->addShader(fragmentShader);
	}
	else if (computeOutputs.size() == 1)
	{
		const auto defaultBuiltInResource = getDefaultBuiltInResource();

		cx.getEmitter().emit(cx, computeOutputs[0]);

		GlslRequirements computeRequirements = cx.requirements();

		const auto& layout = cx.getLayout();
		const char* computeShaderText = strdup(wstombs(cx.getComputeShader().getGeneratedShader(layout, computeRequirements)).c_str());

		// Compute shader.
		computeShader = new glslang::TShader(EShLangCompute);
		computeShader->setStrings(&computeShaderText, 1);
		computeShader->setEntryPoint("main");

		bool computeResult = computeShader->parse(&defaultBuiltInResource, 100, false, (EShMessages)(EShMsgVulkanRules | EShMsgSpvRules));

		if (computeShader->getInfoLog())
		{
			std::wstring info = trim(mbstows(computeShader->getInfoLog()));
			if (!info.empty())
				log::info << info << Endl;
		}

#if defined(_DEBUG)
		if (computeShader->getInfoDebugLog())
			log::info << mbstows(computeShader->getInfoDebugLog()) << Endl;
#endif

		if (!computeResult)
			return nullptr;

		program->addShader(computeShader);
	}
	else
	{
		log::error << L"Unable to generate Vulkan GLSL shader; incorrect number of outputs." << Endl;
		return nullptr;
	}

	// Link program shaders.
	if (!program->link(EShMsgDefault))
		return nullptr;

	Ref< ProgramResourceVk > programResource = new ProgramResourceVk();

	// Output render state.
	programResource->m_renderState = cx.getRenderState();

	// Generate SPIR-V from program AST.
	auto vsi = program->getIntermediate(EShLangVertex);
	if (vsi != nullptr)
	{
		std::vector< uint32_t > vs;
		glslang::GlslangToSpv(*vsi, vs);
		programResource->m_vertexShader = AlignedVector< uint32_t >(vs.begin(), vs.end());
		performOptimization(programResource->m_vertexShader);
	}

	auto fsi = program->getIntermediate(EShLangFragment);
	if (fsi != nullptr)
	{
		std::vector< uint32_t > fs;
		glslang::GlslangToSpv(*fsi, fs);
		programResource->m_fragmentShader = AlignedVector< uint32_t >(fs.begin(), fs.end());
		performOptimization(programResource->m_fragmentShader);
	}

	auto csi = program->getIntermediate(EShLangCompute);
	if (csi != nullptr)
	{
		std::vector< uint32_t > cs;
		glslang::GlslangToSpv(*csi, cs);
		programResource->m_computeShader = AlignedVector< uint32_t >(cs.begin(), cs.end());
		performOptimization(programResource->m_computeShader);
	}

	// Map parameters to uniforms.
	struct ParameterMapping
	{
		uint32_t buffer;
		uint32_t offset;
		uint32_t length;
	};
	std::map< std::wstring, ParameterMapping > parameterMapping;

	for (auto resource : cx.getLayout().get())
	{
		if (const auto sampler = dynamic_type_cast< const GlslSampler* >(resource))
		{
			programResource->m_samplers.push_back(ProgramResourceVk::SamplerDesc(
				sampler->getBinding(),
				sampler->getState()
			));
		}
		else if (const auto texture = dynamic_type_cast< const GlslTexture* >(resource))
		{
			auto& pm = parameterMapping[texture->getName()];
			pm.buffer = texture->getBinding();
			pm.offset = (uint32_t)programResource->m_textures.size();
			pm.length = 0;

			programResource->m_textures.push_back(ProgramResourceVk::TextureDesc(
				texture->getName(),
				texture->getBinding()
			));
		}
		else if (const auto image = dynamic_type_cast< const GlslImage* >(resource))
		{
			auto& pm = parameterMapping[image->getName()];
			pm.buffer = image->getBinding();
			pm.offset = (uint32_t)programResource->m_textures.size();
			pm.length = 0;

			programResource->m_textures.push_back(ProgramResourceVk::TextureDesc(
				image->getName(),
				image->getBinding()
			));
		}
		else if (const auto uniformBuffer = dynamic_type_cast< const GlslUniformBuffer* >(resource))
		{
			uint32_t size = 0;
			for (auto uniform : uniformBuffer->get())
			{
				if (uniform.length > 1)
					size = alignUp(size, 4);

				auto& pm = parameterMapping[uniform.name];
				pm.buffer = uniformBuffer->getBinding();
				pm.offset = size;
				pm.length = glsl_type_width(uniform.type) * uniform.length;

				size += glsl_type_width(uniform.type) * uniform.length;
			}
			programResource->m_uniformBufferSizes[uniformBuffer->getBinding()] = size;
		}
		else if (const auto storageBuffer = dynamic_type_cast< const GlslStorageBuffer* >(resource))
		{
			auto& pm = parameterMapping[storageBuffer->getName()];
			pm.buffer = storageBuffer->getBinding();
			pm.offset = (uint32_t)programResource->m_sbuffers.size();
			pm.length = 0;

			programResource->m_sbuffers.push_back(ProgramResourceVk::SBufferDesc(
				storageBuffer->getName(),
				storageBuffer->getBinding()
			));
		}
	}

	for (auto p : cx.getParameters())
	{
		if (p.type <= PtMatrix)
		{
			auto it = parameterMapping.find(p.name);
			if (it == parameterMapping.end())
				continue;

			const auto& pm = it->second;

			programResource->m_parameters.push_back(ProgramResourceVk::ParameterDesc(
				p.name,
				pm.buffer,
				pm.offset,
				pm.length
			));
		}
		else if (p.type >= PtTexture2D && p.type <= PtTextureCube)
		{
			auto it = parameterMapping.find(p.name);
			if (it == parameterMapping.end())
				continue;

			const auto& pm = it->second;

			programResource->m_parameters.push_back(ProgramResourceVk::ParameterDesc(
				p.name,
				pm.buffer,
				pm.offset,
				pm.length
			));
		}
		else if (p.type >= PtStructBuffer)
		{
			auto it = parameterMapping.find(p.name);
			if (it == parameterMapping.end())
				continue;

			const auto& pm = it->second;

			programResource->m_parameters.push_back(ProgramResourceVk::ParameterDesc(
				p.name,
				pm.buffer,
				pm.offset,
				pm.length
			));
		}
	}

	// Calculate hash of renderstate and shaders.
	Adler32 checksum;
	checksum.begin();
	checksum.feed(cx.getRenderState());
	checksum.feed(programResource->m_vertexShader.c_ptr(), programResource->m_vertexShader.size() * sizeof(uint32_t));
	checksum.feed(programResource->m_fragmentShader.c_ptr(), programResource->m_fragmentShader.size() * sizeof(uint32_t));
	checksum.feed(programResource->m_computeShader.c_ptr(), programResource->m_computeShader.size() * sizeof(uint32_t));
	checksum.end();
	programResource->m_hash = checksum.get();

	// \note Need to delete program before shaders due to glslang weirdness.
	delete program;
	delete fragmentShader;
	delete vertexShader;
	delete computeShader;

	return programResource;
}

bool ProgramCompilerVk::generate(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	const std::wstring& name,
	int32_t optimize,
	std::wstring& outVertexShader,
	std::wstring& outPixelShader,
	std::wstring& outComputeShader
) const
{
	RefArray< VertexOutput > vertexOutputs;
	RefArray< PixelOutput > pixelOutputs;
	RefArray< ComputeOutput > computeOutputs;

	shaderGraph->findNodesOf< VertexOutput >(vertexOutputs);
	shaderGraph->findNodesOf< PixelOutput >(pixelOutputs);
	shaderGraph->findNodesOf< ComputeOutput >(computeOutputs);

	GlslContext cx(shaderGraph);

	if (vertexOutputs.size() == 1 && pixelOutputs.size() == 1)
	{
		cx.getEmitter().emit(cx, pixelOutputs[0]);
		cx.getEmitter().emit(cx, vertexOutputs[0]);
	}
	else if (computeOutputs.size() == 1)
	{
		cx.getEmitter().emit(cx, computeOutputs[0]);
	}
	else
	{
		log::error << L"Unable to generate Vulkan GLSL shader; incorrect number of outputs" << Endl;
		return false;
	}

	const auto& layout = cx.getLayout();

	StringOutputStream ss;
	for (auto resource : layout.get())
	{
		ss << L"// Layout" << Endl;
		if (const auto sampler = dynamic_type_cast< const GlslSampler* >(resource))
		{
			ss << L"// [" << sampler->getBinding() << L"] = sampler" << Endl;
			ss << L"//   .name = \"" << sampler->getName() << L"\"" << Endl;
		}
		else if (const auto texture = dynamic_type_cast< const GlslTexture* >(resource))
		{
			ss << L"// [" << texture->getBinding() << L"] = texture" << Endl;
			ss << L"//   .name = \"" << texture->getName() << L"\"" << Endl;
			ss << L"//   .type = " << int32_t(texture->getUniformType()) << Endl;
		}
		else if (const auto uniformBuffer = dynamic_type_cast< const GlslUniformBuffer* >(resource))
		{
			ss << L"// [" << uniformBuffer->getBinding() << L"] = uniform buffer" << Endl;
			ss << L"//   .name = \"" << uniformBuffer->getName() << L"\"" << Endl;
			ss << L"//   .uniforms = {" << Endl;
			for (auto uniform : uniformBuffer->get())
			{
				ss << L"//      " << int32_t(uniform.type) << L" \"" << uniform.name << L"\" " << uniform.length << Endl;
			}
			ss << L"//   }" << Endl;
		}
		else if (const auto image = dynamic_type_cast< const GlslImage* >(resource))
		{
			ss << L"// [" << image->getBinding() << L"] = image" << Endl;
			ss << L"//   .name = \"" << image->getName() << L"\"" << Endl;
		}
		else if (const auto storageBuffer = dynamic_type_cast< const GlslStorageBuffer* >(resource))
		{
			ss << L"// [" << storageBuffer->getBinding() << L"] = storage buffer" << Endl;
			ss << L"//   .name = \"" << storageBuffer->getName() << L"\"" << Endl;
			ss << L"//   .elements = {" << Endl;
			for (auto element : storageBuffer->get())
			{
				ss << L"//      " << int32_t(element.type) << L" \"" << element.name << Endl;
			}
			ss << L"//   }" << Endl;
		}
	}

	GlslRequirements requirements = cx.requirements();

	// Vertex
	{
		StringOutputStream vss;
		vss << cx.getVertexShader().getGeneratedShader(layout, requirements);
		vss << Endl;
		vss << ss.str();
		vss << Endl;
		outVertexShader = vss.str();
	}

	// Pixel
	{
		StringOutputStream fss;
		fss << cx.getFragmentShader().getGeneratedShader(layout, requirements);
		fss << Endl;
		fss << ss.str();
		fss << Endl;
		outPixelShader = fss.str();
	}

	// Compute
	{
		StringOutputStream css;
		css << cx.getComputeShader().getGeneratedShader(layout, requirements);
		css << Endl;
		css << ss.str();
		css << Endl;
		outComputeShader = css.str();
	}

	return true;
}

	}
}
