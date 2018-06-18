/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include <sstream>

#if defined(_WIN32)
#	include <glslang/Include/ShHandle.h>
#	include <glslang/Include/revision.h>
#	include <glslang/Public/ShaderLang.h>
#	include <SPIRV/GlslangToSpv.h>
#	include <SPIRV/GLSL.std.450.h>
#	include <SPIRV/doc.h>
#	include <SPIRV/disassemble.h>
#endif

#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/Split.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"

#include "Render/Vulkan/ProgramCompilerVk.h"
#include "Render/Vulkan/ProgramResourceVk.h"
#include "Render/Vulkan/Glsl/GlslContext.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

#if defined(_WIN32)
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
#endif

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerVk", 0, ProgramCompilerVk, IProgramCompiler)

ProgramCompilerVk::ProgramCompilerVk()
{
#if defined(_WIN32)
	static bool s_initialized = false;
	if (!s_initialized)
	{
		glslang::InitializeProcess();
		s_initialized = true;
	}
#endif
}

const wchar_t* ProgramCompilerVk::getPlatformSignature() const
{
	return L"Vulkan GLSL";
}

Ref< ProgramResource > ProgramCompilerVk::compile(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	int32_t optimize,
	bool validate,
	Stats* outStats
) const
{
#if defined(_WIN32)
	RefArray< VertexOutput > vertexOutputs;
	RefArray< PixelOutput > pixelOutputs;

	shaderGraph->findNodesOf< VertexOutput >(vertexOutputs);
	shaderGraph->findNodesOf< PixelOutput >(pixelOutputs);

	if (vertexOutputs.size() != 1 || pixelOutputs.size() != 1)
	{
		log::error << L"Unable to generate Vulkan GLSL shader; incorrect number of outputs" << Endl;
		return 0;
	}

	GlslContext cx(shaderGraph);
	cx.getEmitter().emit(cx, pixelOutputs[0]);
	cx.getEmitter().emit(cx, vertexOutputs[0]);

	const char* vertexShaderText = strdup(wstombs(cx.getVertexShader().getGeneratedShader()).c_str());
	const char* fragmentShaderText = strdup(wstombs(cx.getFragmentShader().getGeneratedShader()).c_str());

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
		return 0;
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
		return 0;
	}

	program->addShader(fragmentShader);

	// Link program shaders.
	if (!program->link(EShMsgDefault))
	{
		log::error << L"Failed to compile shader; Program link failed." << Endl;
		return 0;
	}

	Ref< ProgramResourceVk > programResource = new ProgramResourceVk();

	// Output render state.
	programResource->m_renderState = cx.getRenderState();

	// Generate SPIR-V from program AST.
	glslang::GlslangToSpv(*program->getIntermediate(EShLangVertex), programResource->m_vertexShader);
	glslang::GlslangToSpv(*program->getIntermediate(EShLangFragment), programResource->m_fragmentShader);

	// Setup parameter mapping to uniforms.
	const uint32_t scalarTypeSize[] = { 1, 4, 16, 0, 0, 0 };
	std::map< std::wstring, uint32_t > pm;

	uint32_t offset = 0;
	for (auto p : cx.getParameters())
	{
		if (p.type <= PtMatrix)
		{
			programResource->m_parameters.push_back(ProgramResourceVk::ParameterDesc(
				p.name,
				offset,
				scalarTypeSize[p.type] * p.length
			));
			pm[p.name] = offset;
			offset = alignUp(offset + scalarTypeSize[p.type] * p.length, 4);
		}
	}
	programResource->m_parameterScalarSize = offset;

	for (auto u : cx.getVertexShader().getUniforms())
	{
		const GlslContext::Parameter* parameter = cx.getParameter(u);
		if (parameter)
		{
			ProgramResourceVk::UniformBufferDesc& ubd = programResource->m_vertexUniformBuffers[parameter->frequency];

			if (parameter->length > 1)
				ubd.size = alignUp(ubd.size, 4);

			ubd.parameters.push_back(ProgramResourceVk::ParameterMappingDesc(
				ubd.size,
				pm[parameter->name],
				scalarTypeSize[parameter->type] * parameter->length
			));

			ubd.size += scalarTypeSize[parameter->type] * parameter->length;
		}
	}
	
	for (auto u : cx.getFragmentShader().getUniforms())
	{
		const GlslContext::Parameter* parameter = cx.getParameter(u);
		if (parameter)
		{
			ProgramResourceVk::UniformBufferDesc& ubd = programResource->m_fragmentUniformBuffers[parameter->frequency];

			if (parameter->length > 1)
				ubd.size = alignUp(ubd.size, 4);

			ubd.parameters.push_back(ProgramResourceVk::ParameterMappingDesc(
				ubd.size,
				pm[parameter->name],
				scalarTypeSize[parameter->type] * parameter->length
			));

			ubd.size += scalarTypeSize[parameter->type] * parameter->length;
		}
	}

	/*
	// Disassemble SPIR-V.
	std::ostringstream vos;
	spv::Disassemble(vos, programResource->m_vertexShader);
	log::info << L"Vertex SPIRV:" << Endl;
	log::info << mbstows(vos.str()) << Endl;

	std::ostringstream fos;
	spv::Disassemble(fos, programResource->m_fragmentShader);
	log::info << L"Fragment SPIRV:" << Endl;
	log::info << mbstows(fos.str()) << Endl;
	*/

	// \note Need to delete program before shaders due to glslang weirdness.
	delete program;
	delete fragmentShader;
	delete vertexShader;

	return programResource;
#else
	return 0;
#endif
}

bool ProgramCompilerVk::generate(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
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

	outVertexShader = cx.getVertexShader().getGeneratedShader();
	outPixelShader = cx.getFragmentShader().getGeneratedShader();
	return true;
}

	}
}
