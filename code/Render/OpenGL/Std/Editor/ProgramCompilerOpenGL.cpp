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

		const auto defaultBuiltInResource = getDefaultBuiltInResource();
		bool vertexResult = vertexShader->parse(&defaultBuiltInResource, 100, false, (EShMessages)(EShMsgSpvRules));

		if (vertexShader->getInfoLog())
			log::info << mbstows(vertexShader->getInfoLog()) << Endl;

#	if defined(_DEBUG)
		if (vertexShader->getInfoDebugLog())
			log::info << mbstows(vertexShader->getInfoDebugLog()) << Endl;
#	endif

		if (!vertexResult)
			return nullptr;

		program->addShader(vertexShader);

		fragmentShader = new glslang::TShader(EShLangFragment);
		fragmentShader->setStrings(&fragmentShaderText, 1);
		fragmentShader->setEntryPoint("main");

		bool fragmentResult = fragmentShader->parse(&defaultBuiltInResource, 100, false, (EShMessages)(EShMsgSpvRules));

		if (fragmentShader->getInfoLog())
			log::info << mbstows(fragmentShader->getInfoLog()) << Endl;

#	if defined(_DEBUG)
		if (fragmentShader->getInfoDebugLog())
			log::info << mbstows(fragmentShader->getInfoDebugLog()) << Endl;
#	endif

		if (!fragmentResult)
			return nullptr;

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

		const auto defaultBuiltInResource = getDefaultBuiltInResource();
		bool vertexResult = computeShader->parse(&defaultBuiltInResource, 100, false, (EShMessages)(EShMsgSpvRules));

		if (computeShader->getInfoLog())
			log::info << mbstows(computeShader->getInfoLog()) << Endl;

#	if defined(_DEBUG)
		if (computeShader->getInfoDebugLog())
			log::info << mbstows(computeShader->getInfoDebugLog()) << Endl;
#	endif

		if (!vertexResult)
			return nullptr;

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
