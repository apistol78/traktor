#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Render/Ps3/PlatformPs3.h"
#include "Render/Ps3/ProgramPs3.h"
#include "Render/Ps3/ProgramResourcePs3.h"
#include "Render/Ps3/SimpleTexturePs3.h"
#include "Render/Ps3/RenderTargetPs3.h"
#include "Render/Ps3/LocalMemoryAllocator.h"

using namespace cell::Gcm;

namespace traktor
{
	namespace render
	{
		namespace
		{

int buildParameterMap(
	CGprogram program,
	std::map< handle_t, ProgramPs3::Parameter >& outParameterMap
)
{
	uint32_t parameterCount = cellGcmCgGetCountParameter(program);

	for (uint32_t i = 0; i < parameterCount; ++i)
	{
		CGparameter parameter = cellGcmCgGetIndexParameter(program, i);

		const char* parameterName = cellGcmCgGetParameterName(program, parameter);
		T_ASSERT (parameterName);

		log::debug << L"Parameter " << i << L" \"" << mbstows(parameterName) << L"\"" << Endl;
	}

	return 0;

	/*
	RefArray< Uniform > uniformNodes;
	RefArray< IndexedUniform > indexedUniformNodes;
	RefArray< Sampler > samplerNodes;

	shaderGraph->findNodesOf< Uniform >(uniformNodes);
	shaderGraph->findNodesOf< IndexedUniform >(indexedUniformNodes);
	shaderGraph->findNodesOf< Sampler >(samplerNodes);

	int offset = 0;

	for (RefArray< Uniform >::iterator i = uniformNodes.begin(); i != uniformNodes.end(); ++i)
	{
		std::wstring parameterName = (*i)->getParameterName();

		CGparameter parameter = cellGcmCgGetNamedParameter(program, wstombs(parameterName).c_str());
		if (!parameter)
			continue;

		outParameterMap[parameterName].parameters.push_back(parameter);
		outParameterMap[parameterName].sampler = false;
		outParameterMap[parameterName].offset = offset;

		switch ((*i)->getParameterType())
		{
		case PtScalar:
			outParameterMap[parameterName].stride = 1;
			offset += 1;
			break;

		case PtVector:
			outParameterMap[parameterName].stride = 4;
			offset += 4;
			break;

		case PtMatrix:
			outParameterMap[parameterName].stride = 4 * 4;
			offset += 4 * 4;
			break;
		}
	}

	for (RefArray< IndexedUniform >::iterator i = indexedUniformNodes.begin(); i != indexedUniformNodes.end(); ++i)
	{
		for (int j = 0; j < (*i)->getLength(); ++j)
		{
			std::wstring parameterName = (*i)->getParameterName() + L"[" + toString(j) + L"]";

			CGparameter parameter = cellGcmCgGetNamedParameter(program, wstombs(parameterName).c_str());
			if (!parameter)
				break;

			outParameterMap[parameterName].parameters.push_back(parameter);

			if (j == 0)
			{
				outParameterMap[parameterName].sampler = false;
				outParameterMap[parameterName].offset = offset;
			}

			switch ((*i)->getParameterType())
			{
			case PtScalar:
				outParameterMap[parameterName].stride = 1;
				offset += 1;
				break;

			case PtVector:
				outParameterMap[parameterName].stride = 4;
				offset += 4;
				break;

			case PtMatrix:
				outParameterMap[parameterName].stride = 4 * 4;
				offset += 4 * 4;
				break;
			}
		}
	}	

	for (RefArray< Sampler >::iterator i = samplerNodes.begin(); i != samplerNodes.end(); ++i)
	{
		std::wstring parameterName = L"SamplerTexture_" + (*i)->getParameterName();

		CGparameter parameter = cellGcmCgGetNamedParameter(program, wstombs(parameterName).c_str());
		if (!parameter)
			continue;

		CGresource resource = cellGcmCgGetParameterResource(program, parameter);
		if (resource < CG_TEXUNIT0 || resource > CG_TEXUNIT15)
			continue;

		outParameterMap[parameterName].parameters.push_back(parameter);
		outParameterMap[parameterName].sampler = true;
		outParameterMap[parameterName].stage = resource - CG_TEXUNIT0;
	}

	return offset;
	*/
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramPs3", ProgramPs3, IProgram)

ProgramPs3::ProgramPs3()
:	m_vertexProgram(0)
,	m_pixelProgram(0)
,	m_vertexShaderUCodeSize(0)
,	m_vertexShaderUCode(0)
,	m_pixelShaderUCodeSize(0)
,	m_pixelShaderUCode(0)
,	m_pixelShaderOffset(0)
{
	std::memset(m_pixelTextures, 0, sizeof(m_pixelTextures));
}

ProgramPs3::~ProgramPs3()
{
	destroy();
}

bool ProgramPs3::create(const ProgramResourcePs3* resource)
{
	T_ASSERT (resource);

	int size;
	int ret;

	m_resource = resource;
	m_vertexProgram = (CGprogram)sceCgcGetBinData(resource->getVertexShaderBin());
	m_pixelProgram = (CGprogram)sceCgcGetBinData(resource->getPixelShaderBin());

	cellGcmCgInitProgram(m_vertexProgram);
	cellGcmCgInitProgram(m_pixelProgram);

	cellGcmCgGetUCode(m_vertexProgram, &m_vertexShaderUCode, &m_vertexShaderUCodeSize);
	cellGcmCgGetUCode(m_pixelProgram, &m_pixelShaderUCode, &m_pixelShaderUCodeSize);

	void* aligned = LocalMemoryAllocator::getInstance().allocAlign(m_pixelShaderUCodeSize, 64);
	std::memcpy(aligned, m_pixelShaderUCode, m_pixelShaderUCodeSize);
	m_pixelShaderUCode = aligned;

	ret = cellGcmAddressToOffset(m_pixelShaderUCode, &m_pixelShaderOffset);
	if (ret != CELL_OK)
	{
		log::error << L"Create shader failed, unable to get offset to pixel shader" << Endl;
		return false;
	}

	size = buildParameterMap(m_vertexProgram, m_vertexParameterMap);
	m_vertexParameters.resize(size);

	size = buildParameterMap(m_pixelProgram, m_pixelParameterMap);
	m_pixelParameters.resize(size);

	//m_renderState = renderState;
	return true;
}

void ProgramPs3::destroy()
{
	if (m_pixelShaderUCode)
	{
		LocalMemoryAllocator::getInstance().free(m_pixelShaderUCode);

		m_pixelShaderUCode = 0;
		m_pixelShaderOffset = 0;
	}
}

void ProgramPs3::setFloatParameter(handle_t handle, float param)
{
	setFloatArrayParameter(handle, &param, 1);
}

void ProgramPs3::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	std::map< handle_t, Parameter >::iterator i = m_vertexParameterMap.find(handle);
	if (i != m_vertexParameterMap.end())
	{
		T_ASSERT (!i->second.sampler);
		std::memcpy(&m_vertexParameters[i->second.offset], param, length * sizeof(float));
	}

	std::map< handle_t, Parameter >::iterator j = m_pixelParameterMap.find(handle);
	if (j != m_pixelParameterMap.end())
	{
		T_ASSERT (!j->second.sampler);
		std::memcpy(&m_pixelParameters[j->second.offset], param, length * sizeof(float));
	}
}

void ProgramPs3::setVectorParameter(handle_t handle, const Vector4& param)
{
	setFloatArrayParameter(handle, (const float*)&param, 4);
}

void ProgramPs3::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	setFloatArrayParameter(handle, (const float*)param, 4 * length);
}

void ProgramPs3::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	setFloatArrayParameter(handle, (const float*)&param, 4 * 4);
}

void ProgramPs3::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	setFloatArrayParameter(handle, (const float*)param, 4 * 4 * length);
}

void ProgramPs3::setSamplerTexture(handle_t handle, ITexture* texture)
{
	std::map< handle_t, Parameter >::iterator i = m_pixelParameterMap.find(handle);
	if (i != m_pixelParameterMap.end())
	{
		T_ASSERT (i->second.sampler);
		m_pixelTextures[i->second.stage] = texture;
	}
}

void ProgramPs3::setStencilReference(uint32_t stencilReference)
{
}

bool ProgramPs3::isOpaque() const
{
	return true;
}

void ProgramPs3::bind()
{
	cellGcmSetCullFaceEnable(m_renderState.cullFaceEnable);
	cellGcmSetCullFace(gCellGcmCurrentContext, m_renderState.cullFace);
	cellGcmSetBlendEnable(gCellGcmCurrentContext, m_renderState.blendEnable);
	cellGcmSetBlendEquation(gCellGcmCurrentContext, m_renderState.blendEquation, CELL_GCM_FUNC_ADD);
	cellGcmSetBlendFunc(gCellGcmCurrentContext, m_renderState.blendFuncSrc, m_renderState.blendFuncDest, CELL_GCM_ONE, CELL_GCM_ZERO);
	cellGcmSetDepthTestEnable(gCellGcmCurrentContext, m_renderState.depthTestEnable);
	cellGcmSetColorMask(gCellGcmCurrentContext, m_renderState.colorMask);
	cellGcmSetDepthMask(gCellGcmCurrentContext, m_renderState.depthMask);
	cellGcmSetDepthFunc(gCellGcmCurrentContext, m_renderState.depthFunc);
	cellGcmSetAlphaTestEnable(gCellGcmCurrentContext, CELL_GCM_FALSE); //m_renderState.alphaTestEnable);
	cellGcmSetAlphaFunc(gCellGcmCurrentContext, m_renderState.alphaFunc, m_renderState.alphaRef);

	cellGcmSetVertexProgram(m_vertexProgram, m_vertexShaderUCode);

	for (std::map< handle_t, Parameter >::iterator i = m_vertexParameterMap.begin(); i != m_vertexParameterMap.end(); ++i)
	{
		if (i->second.sampler)
			continue;

		for (uint32_t j = 0; j < i->second.parameters.size(); ++j)
		{
			cellGcmSetVertexProgramParameter(
				gCellGcmCurrentContext,
				i->second.parameters[j],
				&m_vertexParameters[i->second.offset + j * i->second.stride]
			);
		}
	}

	cellGcmSetFragmentProgram(m_pixelProgram, m_pixelShaderOffset);

	for (std::map< handle_t, Parameter >::iterator i = m_pixelParameterMap.begin(); i != m_pixelParameterMap.end(); ++i)
	{
		if (i->second.sampler)
			continue;

		for (uint32_t j = 0; j < i->second.parameters.size(); ++j)
		{
			cellGcmSetFragmentProgramParameter(
				gCellGcmCurrentContext,
				m_pixelProgram,
				i->second.parameters[j],
				&m_pixelParameters[i->second.offset + j * i->second.stride],
				m_pixelShaderOffset
			);
		}
	}

	for (int i = 0; i < 16; ++i)
	{
		if (m_pixelTextures[i])
		{
			if (is_a< SimpleTexturePs3 >(m_pixelTextures[i]))
				static_cast< SimpleTexturePs3* >(m_pixelTextures[i])->bind(i, *(SamplerState*)0);
			else if (is_a< RenderTargetPs3 >(m_pixelTextures[i]))
				static_cast< RenderTargetPs3* >(m_pixelTextures[i])->bind(i, *(SamplerState*)0);
		}
		else
			cellGcmSetTextureControl(
				gCellGcmCurrentContext,
				i,
				CELL_GCM_FALSE,
				0,
				0,
				0
			);
	}
}

	}
}
