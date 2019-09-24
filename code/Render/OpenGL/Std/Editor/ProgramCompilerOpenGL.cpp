#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/DeepHash.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/OpenGL/Std/ProgramResourceOpenGL.h"
#include "Render/OpenGL/Std/Editor/ProgramCompilerOpenGL.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslContext.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslImage.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslSampler.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslStorageBuffer.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslTexture.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslUniformBuffer.h"

#if defined(T_ENABLE_GLSL_VERIFY)
#	include <glslang/Include/ShHandle.h>
#	include <glslang/Include/revision.h>
#	include <glslang/Public/ShaderLang.h>
#endif

namespace traktor
{
	namespace render
	{
		namespace
		{

#if defined(T_ENABLE_GLSL_VERIFY)

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

#endif

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerOpenGL", 0, ProgramCompilerOpenGL, IProgramCompiler)

ProgramCompilerOpenGL::ProgramCompilerOpenGL()
{
#if defined(T_ENABLE_GLSL_VERIFY)
	static bool s_initialized = false;
	if (!s_initialized)
	{
		glslang::InitializeProcess();
		s_initialized = true;
	}
#endif
}

const wchar_t* ProgramCompilerOpenGL::getRendererSignature() const
{
	return L"OpenGL";
}

Ref< ProgramResource > ProgramCompilerOpenGL::compile(
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

#if defined(T_ENABLE_GLSL_VERIFY)
	glslang::TProgram* program = new glslang::TProgram();
	glslang::TShader* vertexShader = nullptr;
	glslang::TShader* fragmentShader = nullptr;
	glslang::TShader* computeShader = nullptr;
#endif

	Ref< ProgramResourceOpenGL > programResource = new ProgramResourceOpenGL();

	if (vertexOutputs.size() == 1 && pixelOutputs.size() == 1)
	{
		cx.getEmitter().emit(cx, pixelOutputs[0]);
		cx.getEmitter().emit(cx, vertexOutputs[0]);

		const auto& layout = cx.getLayout();
		programResource->m_vertexShader = wstombs(cx.getVertexShader().getGeneratedShader(layout));
		programResource->m_fragmentShader = wstombs(cx.getFragmentShader().getGeneratedShader(layout));

#if defined(T_ENABLE_GLSL_VERIFY)
		const char* vertexShaderText = strdup(programResource->m_vertexShader.c_str());
		const char* fragmentShaderText = strdup(programResource->m_fragmentShader.c_str());

		vertexShader = new glslang::TShader(EShLangVertex);
		vertexShader->setStrings(&vertexShaderText, 1);
		vertexShader->setEntryPoint("main");

		bool vertexResult = vertexShader->parse(&c_defaultTBuiltInResource, 100, false, (EShMessages)(EShMsgSpvRules));

		if (vertexShader->getInfoLog())
			log::info << mbstows(vertexShader->getInfoLog()) << Endl;

#	if defined(_DEBUG)
		if (vertexShader->getInfoDebugLog())
			log::info << mbstows(vertexShader->getInfoDebugLog()) << Endl;
#	endif

		if (!vertexResult)
		{
			log::error << L"Failed to compile shader; Failed to parse vertex shader." << Endl;
			return nullptr;
		}

		program->addShader(vertexShader);

		fragmentShader = new glslang::TShader(EShLangFragment);
		fragmentShader->setStrings(&fragmentShaderText, 1);
		fragmentShader->setEntryPoint("main");

		bool fragmentResult = fragmentShader->parse(&c_defaultTBuiltInResource, 100, false, (EShMessages)(EShMsgSpvRules));

		if (fragmentShader->getInfoLog())
			log::info << mbstows(fragmentShader->getInfoLog()) << Endl;

#	if defined(_DEBUG)
		if (fragmentShader->getInfoDebugLog())
			log::info << mbstows(fragmentShader->getInfoDebugLog()) << Endl;
#	endif

		if (!fragmentResult)
		{
			log::error << L"Failed to compile shader; Failed to parse fragment shader." << Endl;
			return nullptr;
		}

		program->addShader(fragmentShader);
#endif
	}
	else if (computeOutputs.size() == 1)
	{
		cx.getEmitter().emit(cx, computeOutputs[0]);

		const auto& layout = cx.getLayout();
		programResource->m_computeShader = wstombs(cx.getComputeShader().getGeneratedShader(layout));

#if defined(T_ENABLE_GLSL_VERIFY)
		const char* computeShaderText = strdup(programResource->m_computeShader.c_str());

		computeShader = new glslang::TShader(EShLangCompute);
		computeShader->setStrings(&computeShaderText, 1);
		computeShader->setEntryPoint("main");

		bool vertexResult = computeShader->parse(&c_defaultTBuiltInResource, 100, false, (EShMessages)(EShMsgSpvRules));

		if (computeShader->getInfoLog())
			log::info << mbstows(computeShader->getInfoLog()) << Endl;

#	if defined(_DEBUG)
		if (computeShader->getInfoDebugLog())
			log::info << mbstows(computeShader->getInfoDebugLog()) << Endl;
#	endif

		if (!vertexResult)
		{
			log::error << L"Failed to compile shader; Failed to parse compute shader." << Endl;
			return nullptr;
		}

		program->addShader(computeShader);
#endif	
	}

	programResource->m_renderState = cx.getRenderState();
	programResource->m_texturesCount = 0;
	programResource->m_sbufferCount = 0;

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
			const auto textures = cx.getLayout().get< GlslTexture >();

			auto it = std::find_if(textures.begin(), textures.end(), [&](const GlslTexture* texture) {
				return texture->getName() == sampler->getTextureName();
			});
			if (it == textures.end())
				return nullptr;

			programResource->m_samplers.push_back(ProgramResourceOpenGL::SamplerDesc(
				sampler->getUnit(),
				sampler->getState(),
				(uint32_t)std::distance(textures.begin(), it)
			));
		}
		else if (const auto texture = dynamic_type_cast< const GlslTexture* >(resource))
		{
			auto& pm = parameterMapping[texture->getName()];
			pm.buffer = (int32_t)cx.getLayout().typedIndexOf< GlslTexture >(texture);
			pm.offset = 0;
			pm.length = 0;

			programResource->m_texturesCount++;
		}
		// else if (const auto image = dynamic_type_cast< const GlslImage* >(resource))
		// {
		// 	auto& pm = parameterMapping[image->getName()];
		// 	pm.buffer = image->getBinding();
		// 	pm.offset = 0;
		// 	pm.length = 0;
		// }
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
			pm.buffer = (int32_t)cx.getLayout().typedIndexOf< GlslStorageBuffer >(storageBuffer);
			pm.offset = 0;
			pm.length = 0;

			programResource->m_sbufferCount++;
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

			programResource->m_parameters.push_back(ProgramResourceOpenGL::ParameterDesc(
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

			programResource->m_parameters.push_back(ProgramResourceOpenGL::ParameterDesc(
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

			programResource->m_parameters.push_back(ProgramResourceOpenGL::ParameterDesc(
				p.name,
				pm.buffer,
				pm.offset,
				pm.length
			));
		}
	}

	programResource->m_hash = DeepHash(programResource).get();

	return programResource;
}

bool ProgramCompilerOpenGL::generate(
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
			ss << L"// [" << sampler->getUnit() << L"] = sampler" << Endl;
			ss << L"//   .name = \"" << sampler->getName() << L"\"" << Endl;
		}
		else if (const auto texture = dynamic_type_cast< const GlslTexture* >(resource))
		{
			ss << L"// [] = texture" << Endl;
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
		// else if (const auto image = dynamic_type_cast< const GlslImage* >(resource))
		// {
		// 	ss << L"// [" << image->getBinding() << L"] = image" << Endl;
		// 	ss << L"//   .name = \"" << image->getName() << L"\"" << Endl;
		// }
		// else if (const auto storageBuffer = dynamic_type_cast< const GlslStorageBuffer* >(resource))
		// {
		// 	ss << L"// [" << storageBuffer->getBinding() << L"] = storage buffer" << Endl;
		// 	ss << L"//   .name = \"" << storageBuffer->getName() << L"\"" << Endl;
		// 	ss << L"//   .elements = {" << Endl;
		// 	for (auto element : storageBuffer->get())
		// 	{
		// 		ss << L"//      " << int32_t(element.type) << L" \"" << element.name << Endl;
		// 	}
		// 	ss << L"//   }" << Endl;
		// }
	}

	// Vertex
	{
		StringOutputStream vss;
		vss << cx.getVertexShader().getGeneratedShader(layout);
		vss << Endl;
		vss << ss.str();
		vss << Endl;
		outVertexShader = vss.str();
	}

	// Pixel
	{
		StringOutputStream fss;
		fss << cx.getFragmentShader().getGeneratedShader(layout);
		fss << Endl;
		fss << ss.str();
		fss << Endl;
		outPixelShader = fss.str();
	}

	// Compute
	{
		StringOutputStream css;
		css << cx.getComputeShader().getGeneratedShader(layout);
		css << Endl;
		css << ss.str();
		css << Endl;
		outComputeShader = css.str();
	}

	return true;
}

	}
}
