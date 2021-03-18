#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/DeepHash.h"
#include "Render/Editor/Glsl/GlslContext.h"
#include "Render/Editor/Glsl/GlslImage.h"
#include "Render/Editor/Glsl/GlslSampler.h"
#include "Render/Editor/Glsl/GlslStorageBuffer.h"
#include "Render/Editor/Glsl/GlslTexture.h"
#include "Render/Editor/Glsl/GlslUniformBuffer.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/OpenGL/ES/ProgramResourceOpenGLES.h"
#include "Render/OpenGL/ES/Editor/ProgramCompilerOpenGLES.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerOpenGLES", 0, ProgramCompilerOpenGLES, IProgramCompiler)

const wchar_t* ProgramCompilerOpenGLES::getRendererSignature() const
{
	return L"OpenGL ES";
}

Ref< ProgramResource > ProgramCompilerOpenGLES::compile(
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

	GlslContext cx(shaderGraph, settings, GlslDialect::OpenGL);

	Ref< ProgramResourceOpenGLES > programResource = new ProgramResourceOpenGLES();

	if (vertexOutputs.size() == 1 && pixelOutputs.size() == 1)
	{
		cx.getEmitter().emit(cx, pixelOutputs[0]);
		cx.getEmitter().emit(cx, vertexOutputs[0]);

		GlslRequirements vertexRequirements = cx.requirements();
		GlslRequirements fragmentRequirements = cx.requirements();

		const auto& layout = cx.getLayout();
		programResource->m_vertexShader = wstombs(cx.getVertexShader().getGeneratedShader(settings, layout, vertexRequirements));
		programResource->m_fragmentShader = wstombs(cx.getFragmentShader().getGeneratedShader(settings, layout, fragmentRequirements));
	}
	else if (computeOutputs.size() == 1)
	{
		cx.getEmitter().emit(cx, computeOutputs[0]);

		GlslRequirements computeRequirements = cx.requirements();

		const auto& layout = cx.getLayout();
		programResource->m_computeShader = wstombs(cx.getComputeShader().getGeneratedShader(settings, layout, computeRequirements));
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
			// Get index of sampler among samplers, used to determine which texture unit.
			int32_t textureUnit = cx.getLayout().getLocalIndex(sampler);

			// Find texture resource, calculate index into texture parameter array.
			const auto textures = cx.getLayout().get< GlslTexture >();
			auto it = std::find_if(textures.begin(), textures.end(), [&](const GlslTexture* texture) {
				return texture->getName() == sampler->getTextureName();
			});
			if (it == textures.end())
				return nullptr;

			programResource->m_samplers.push_back(ProgramResourceOpenGLES::SamplerDesc(
				textureUnit,
				sampler->getState(),
				(uint32_t)std::distance(textures.begin(), it)
			));
		}
		else if (const auto texture = dynamic_type_cast< const GlslTexture* >(resource))
		{
			auto& pm = parameterMapping[texture->getName()];
			pm.buffer = cx.getLayout().getLocalIndex(texture);
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
				pm.buffer = uniformBuffer->getBinding(GlslDialect::OpenGL);
				pm.offset = size;
				pm.length = glsl_type_width(uniform.type) * uniform.length;

				size += glsl_type_width(uniform.type) * uniform.length;
			}
			programResource->m_uniformBufferSizes[uniformBuffer->getBinding(GlslDialect::OpenGL)] = size;
		}
		else if (const auto storageBuffer = dynamic_type_cast< const GlslStorageBuffer* >(resource))
		{
			auto& pm = parameterMapping[storageBuffer->getName()];
			pm.buffer = (int32_t)cx.getLayout().getLocalIndex(storageBuffer);
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

			programResource->m_parameters.push_back(ProgramResourceOpenGLES::ParameterDesc(
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

			programResource->m_parameters.push_back(ProgramResourceOpenGLES::ParameterDesc(
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

			programResource->m_parameters.push_back(ProgramResourceOpenGLES::ParameterDesc(
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

bool ProgramCompilerOpenGLES::generate(
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

	GlslContext cx(shaderGraph, settings, GlslDialect::OpenGL);

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
		log::error << L"Unable to generate OpenGL ES GLSL shader; incorrect number of outputs." << Endl;
		return false;
	}

	const auto& layout = cx.getLayout();
	GlslRequirements requirements = cx.requirements();

	StringOutputStream ss;

	// Vertex
	{
		StringOutputStream vss;
		vss << cx.getVertexShader().getGeneratedShader(settings, layout, requirements);
		vss << Endl;
		vss << ss.str();
		vss << Endl;
		outVertexShader = vss.str();
	}

	// Pixel
	{
		StringOutputStream fss;
		fss << cx.getFragmentShader().getGeneratedShader(settings, layout, requirements);
		fss << Endl;
		fss << ss.str();
		fss << Endl;
		outPixelShader = fss.str();
	}

	// Compute
	{
		StringOutputStream css;
		css << cx.getComputeShader().getGeneratedShader(settings, layout, requirements);
		css << Endl;
		css << ss.str();
		css << Endl;
		outComputeShader = css.str();
	}

	return true;
}

	}
}
