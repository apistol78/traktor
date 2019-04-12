#include <cstring>
#include <sstream>

#include <glslang/Include/ShHandle.h>
#include <glslang/Include/revision.h>
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <SPIRV/GLSL.std.450.h>
#include <SPIRV/doc.h>
#include <SPIRV/disassemble.h>

#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/Split.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"

#include "Render/Vulkan/ProgramResourceVk.h"
#include "Render/Vulkan/Editor/ProgramCompilerVk.h"
#include "Render/Vulkan/Editor/Glsl/GlslContext.h"
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

const TBuiltInResource c_defaultTBuiltInResource =
{
	/* .MaxLights = */ 32,
	/* .MaxClipPlanes = */ 6,
	/* .MaxTextureUnits = */ 32,
	/* .MaxTextureCoords = */ 32,
	/* .MaxVertexAttribs = */ 64,
	/* .MaxVertexUniformComponents = */ 4096,
	/* .MaxVaryingFloats = */ 64,
	/* .MaxVertexTextureImageUnits = */ 32,
	/* .MaxCombinedTextureImageUnits = */ 80,
	/* .MaxTextureImageUnits = */ 32,
	/* .MaxFragmentUniformComponents = */ 4096,
	/* .MaxDrawBuffers = */ 32,
	/* .MaxVertexUniformVectors = */ 128,
	/* .MaxVaryingVectors = */ 8,
	/* .MaxFragmentUniformVectors = */ 16,
	/* .MaxVertexOutputVectors = */ 16,
	/* .MaxFragmentInputVectors = */ 15,
	/* .MinProgramTexelOffset = */ -8,
	/* .MaxProgramTexelOffset = */ 7,
	/* .MaxClipDistances = */ 8,
	/* .MaxComputeWorkGroupCountX = */ 65535,
	/* .MaxComputeWorkGroupCountY = */ 65535,
	/* .MaxComputeWorkGroupCountZ = */ 65535,
	/* .MaxComputeWorkGroupSizeX = */ 1024,
	/* .MaxComputeWorkGroupSizeY = */ 1024,
	/* .MaxComputeWorkGroupSizeZ = */ 64,
	/* .MaxComputeUniformComponents = */ 1024,
	/* .MaxComputeTextureImageUnits = */ 16,
	/* .MaxComputeImageUniforms = */ 8,
	/* .MaxComputeAtomicCounters = */ 8,
	/* .MaxComputeAtomicCounterBuffers = */ 1,
	/* .MaxVaryingComponents = */ 60,
	/* .MaxVertexOutputComponents = */ 64,
	/* .MaxGeometryInputComponents = */ 64,
	/* .MaxGeometryOutputComponents = */ 128,
	/* .MaxFragmentInputComponents = */ 128,
	/* .MaxImageUnits = */ 8,
	/* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
	/* .MaxCombinedShaderOutputResources = */ 8,
	/* .MaxImageSamples = */ 0,
	/* .MaxVertexImageUniforms = */ 0,
	/* .MaxTessControlImageUniforms = */ 0,
	/* .MaxTessEvaluationImageUniforms = */ 0,
	/* .MaxGeometryImageUniforms = */ 0,
	/* .MaxFragmentImageUniforms = */ 8,
	/* .MaxCombinedImageUniforms = */ 8,
	/* .MaxGeometryTextureImageUnits = */ 16,
	/* .MaxGeometryOutputVertices = */ 256,
	/* .MaxGeometryTotalOutputComponents = */ 1024,
	/* .MaxGeometryUniformComponents = */ 1024,
	/* .MaxGeometryVaryingComponents = */ 64,
	/* .MaxTessControlInputComponents = */ 128,
	/* .MaxTessControlOutputComponents = */ 128,
	/* .MaxTessControlTextureImageUnits = */ 16,
	/* .MaxTessControlUniformComponents = */ 1024,
	/* .MaxTessControlTotalOutputComponents = */ 4096,
	/* .MaxTessEvaluationInputComponents = */ 128,
	/* .MaxTessEvaluationOutputComponents = */ 128,
	/* .MaxTessEvaluationTextureImageUnits = */ 16,
	/* .MaxTessEvaluationUniformComponents = */ 1024,
	/* .MaxTessPatchComponents = */ 120,
	/* .MaxPatchVertices = */ 32,
	/* .MaxTessGenLevel = */ 64,
	/* .MaxViewports = */ 16,
	/* .MaxVertexAtomicCounters = */ 0,
	/* .MaxTessControlAtomicCounters = */ 0,
	/* .MaxTessEvaluationAtomicCounters = */ 0,
	/* .MaxGeometryAtomicCounters = */ 0,
	/* .MaxFragmentAtomicCounters = */ 8,
	/* .MaxCombinedAtomicCounters = */ 8,
	/* .MaxAtomicCounterBindings = */ 1,
	/* .MaxVertexAtomicCounterBuffers = */ 0,
	/* .MaxTessControlAtomicCounterBuffers = */ 0,
	/* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
	/* .MaxGeometryAtomicCounterBuffers = */ 0,
	/* .MaxFragmentAtomicCounterBuffers = */ 1,
	/* .MaxCombinedAtomicCounterBuffers = */ 1,
	/* .MaxAtomicCounterBufferSize = */ 16384,
	/* .MaxTransformFeedbackBuffers = */ 4,
	/* .MaxTransformFeedbackInterleavedComponents = */ 64,
	/* .MaxCullDistances = */ 8,
	/* .MaxCombinedClipAndCullDistances = */ 8,
	/* .MaxSamples = */ 4,

	/* .maxMeshOutputVerticesNV; =*/ 0,
    /* .maxMeshOutputPrimitivesNV; =*/ 0,
    /* .maxMeshWorkGroupSizeX_NV; =*/ 0,
    /* .maxMeshWorkGroupSizeY_NV; =*/ 0,
    /* .maxMeshWorkGroupSizeZ_NV; =*/ 0,
    /* .maxTaskWorkGroupSizeX_NV; =*/ 0,
    /* .maxTaskWorkGroupSizeY_NV; =*/ 0,
    /* .maxTaskWorkGroupSizeZ_NV; =*/ 0,
    /* .maxMeshViewCountNV; =*/ 0,

	/* .limits = */ {
		/* .nonInductiveForLoops = */ 1,
		/* .whileLoops = */ 1,
		/* .doWhileLoops = */ 1,
		/* .generalUniformIndexing = */ 1,
		/* .generalAttributeMatrixVectorIndexing = */ 1,
		/* .generalVaryingIndexing = */ 1,
		/* .generalSamplerIndexing = */ 1,
		/* .generalVariableIndexing = */ 1,
		/* .generalConstantMatrixVectorIndexing = */ 1,
    }
};

const uint32_t c_parameterTypeWidths[] = { 1, 4, 16, 0, 0, 0 };

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

const wchar_t* ProgramCompilerVk::getPlatformSignature() const
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

	shaderGraph->findNodesOf< VertexOutput >(vertexOutputs);
	shaderGraph->findNodesOf< PixelOutput >(pixelOutputs);

	if (vertexOutputs.size() != 1 || pixelOutputs.size() != 1)
	{
		log::error << L"Unable to generate Vulkan GLSL shader; incorrect number of outputs" << Endl;
		return nullptr;
	}

	GlslContext cx(shaderGraph);
	cx.getEmitter().emit(cx, pixelOutputs[0]);
	cx.getEmitter().emit(cx, vertexOutputs[0]);

	const auto& layout = cx.getLayout();

	const char* vertexShaderText = strdup(wstombs(cx.getVertexShader().getGeneratedShader(layout)).c_str());
	const char* fragmentShaderText = strdup(wstombs(cx.getFragmentShader().getGeneratedShader(layout)).c_str());

	glslang::TProgram* program = new glslang::TProgram();

	// Vertex shader.
	glslang::TShader* vertexShader = new glslang::TShader(EShLangVertex);
	vertexShader->setStrings(&vertexShaderText, 1);
	vertexShader->setEntryPoint("main");

	bool vertexResult = vertexShader->parse(&c_defaultTBuiltInResource, 100, false, (EShMessages)(EShMsgVulkanRules | EShMsgSpvRules));

	if (vertexShader->getInfoLog())
		log::info << mbstows(vertexShader->getInfoLog()) << Endl;

	if (vertexShader->getInfoDebugLog())
		log::info << mbstows(vertexShader->getInfoDebugLog()) << Endl;

	if (!vertexResult)
	{
		log::error << L"Failed to compile shader; Failed to parse vertex shader." << Endl;
		return nullptr;
	}

	program->addShader(vertexShader);

	// Fragment shader.
	glslang::TShader* fragmentShader = new glslang::TShader(EShLangFragment);
	fragmentShader->setStrings(&fragmentShaderText, 1);
	fragmentShader->setEntryPoint("main");

	bool fragmentResult = fragmentShader->parse(&c_defaultTBuiltInResource, 100, false, (EShMessages)(EShMsgVulkanRules | EShMsgSpvRules));

	if (fragmentShader->getInfoLog())
		log::info << mbstows(fragmentShader->getInfoLog()) << Endl;

	if (fragmentShader->getInfoDebugLog())
		log::info << mbstows(fragmentShader->getInfoDebugLog()) << Endl;

	if (!fragmentResult)
	{
		log::error << L"Failed to compile shader; Failed to parse fragment shader." << Endl;
		return nullptr;
	}

	program->addShader(fragmentShader);

	// Link program shaders.
	if (!program->link(EShMsgDefault))
	{
		log::error << L"Failed to compile shader; Program link failed." << Endl;
		return nullptr;
	}

	Ref< ProgramResourceVk > programResource = new ProgramResourceVk();

	// Output render state.
	programResource->m_renderState = cx.getRenderState();

	// Generate SPIR-V from program AST.
	std::vector< uint32_t > vs;
	glslang::GlslangToSpv(*program->getIntermediate(EShLangVertex), vs);
	programResource->m_vertexShader = AlignedVector< uint32_t >(vs.begin(), vs.end());

	std::vector< uint32_t > fs;
	glslang::GlslangToSpv(*program->getIntermediate(EShLangFragment), fs);
	programResource->m_fragmentShader = AlignedVector< uint32_t >(fs.begin(), fs.end());

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
				texture->getBinding()
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
	Adler32 cs;
	cs.begin();
	cs.feed(cx.getRenderState());
	cs.feed(programResource->m_vertexShader.c_ptr(), programResource->m_vertexShader.size() * sizeof(uint32_t));
	cs.feed(programResource->m_fragmentShader.c_ptr(), programResource->m_fragmentShader.size() * sizeof(uint32_t));
	cs.end();
	programResource->m_hash = cs.get();

	// \note Need to delete program before shaders due to glslang weirdness.
	delete program;
	delete fragmentShader;
	delete vertexShader;

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

	shaderGraph->findNodesOf< VertexOutput >(vertexOutputs);
	shaderGraph->findNodesOf< PixelOutput >(pixelOutputs);

	if (vertexOutputs.size() != 1 || pixelOutputs.size() != 1)
	{
		log::error << L"Unable to generate Vulkan GLSL shader; incorrect number of outputs" << Endl;
		return false;
	}

	GlslContext cx(shaderGraph);
	cx.getEmitter().emit(cx, pixelOutputs[0]);
	cx.getEmitter().emit(cx, vertexOutputs[0]);

	const auto& layout = cx.getLayout();

	const char* vertexShaderText = strdup(wstombs(cx.getVertexShader().getGeneratedShader(layout)).c_str());
	const char* fragmentShaderText = strdup(wstombs(cx.getFragmentShader().getGeneratedShader(layout)).c_str());

	//glslang::TProgram* program = new glslang::TProgram();

	//// Vertex shader.
	//glslang::TShader* vertexShader = new glslang::TShader(EShLangVertex);
	//vertexShader->setStrings(&vertexShaderText, 1);
	//vertexShader->setEntryPoint("main");

	//bool vertexResult = vertexShader->parse(&c_defaultTBuiltInResource, 100, false, (EShMessages)(EShMsgVulkanRules | EShMsgSpvRules));

	//if (vertexShader->getInfoLog())
	//	log::info << mbstows(vertexShader->getInfoLog()) << Endl;

	//if (vertexShader->getInfoDebugLog())
	//	log::info << mbstows(vertexShader->getInfoDebugLog()) << Endl;

	//if (!vertexResult)
	//{
	//	log::error << L"Failed to compile shader; Failed to parse vertex shader." << Endl;
	//	return false;
	//}

	//program->addShader(vertexShader);

	//// Fragment shader.
	//glslang::TShader* fragmentShader = new glslang::TShader(EShLangFragment);
	//fragmentShader->setStrings(&fragmentShaderText, 1);
	//fragmentShader->setEntryPoint("main");

	//bool fragmentResult = fragmentShader->parse(&c_defaultTBuiltInResource, 100, false, (EShMessages)(EShMsgVulkanRules | EShMsgSpvRules));

	//if (fragmentShader->getInfoLog())
	//	log::info << mbstows(fragmentShader->getInfoLog()) << Endl;

	//if (fragmentShader->getInfoDebugLog())
	//	log::info << mbstows(fragmentShader->getInfoDebugLog()) << Endl;

	//if (!fragmentResult)
	//{
	//	log::error << L"Failed to compile shader; Failed to parse fragment shader." << Endl;
	//	return false;
	//}

	//program->addShader(fragmentShader);

	//// Link program shaders.
	//if (!program->link(EShMsgDefault))
	//{
	//	log::error << L"Failed to compile shader; Program link failed." << Endl;
	//	return false;
	//}

	//// Generate SPIR-V from program AST.
	//std::vector< uint32_t > vertexShaderSpv;
	//glslang::GlslangToSpv(*program->getIntermediate(EShLangVertex), vertexShaderSpv);

	//std::vector< uint32_t > fragmentShaderSpv;
	//glslang::GlslangToSpv(*program->getIntermediate(EShLangFragment), fragmentShaderSpv);

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

	// Vertex
	{
		StringOutputStream vss;
		vss << cx.getVertexShader().getGeneratedShader(layout);
		vss << Endl;
		vss << ss.str();
		vss << Endl;

		//std::ostringstream vos;
		//spv::Disassemble(vos, vertexShaderSpv);
		//vss << mbstows(vos.str()) << Endl;

		outVertexShader = vss.str();
	}

	// Pixel
	{
		StringOutputStream fss;
		fss << cx.getFragmentShader().getGeneratedShader(layout);
		fss << Endl;
		fss << ss.str();
		fss << Endl;

		//std::ostringstream fos;
		//spv::Disassemble(fos, fragmentShaderSpv);
		//fss << mbstows(fos.str()) << Endl;

		outPixelShader = fss.str();
	}

	return true;
}

	}
}
