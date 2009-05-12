#include <cell/gcm.h>
#include <Cg/cgc.h>
#include "Render/Ps3/ShaderPs3.h"
#include "Render/Ps3/SimpleTexturePs3.h"
#include "Render/Ps3/RenderTargetPs3.h"
#include "Render/Ps3/LocalMemoryAllocator.h"
#include "Render/ShaderGraph.h"
#include "Render/Nodes.h"
#include "Core/Misc/StringUtils.h"
#include "Core/Log/Log.h"

using namespace cell::Gcm;

namespace traktor
{
	namespace render
	{
		namespace
		{

int buildParameterMap(
	const ShaderGraph* shaderGraph,
	CGprogram program,
	std::map< std::wstring, ShaderPs3::Parameter >& outParameterMap
)
{
	RefList< Uniform > uniformNodes;
	RefList< IndexedUniform > indexedUniformNodes;
	RefList< Sampler > samplerNodes;

	shaderGraph->findNodesOf< Uniform >(uniformNodes);
	shaderGraph->findNodesOf< IndexedUniform >(indexedUniformNodes);
	shaderGraph->findNodesOf< Sampler >(samplerNodes);

	int offset = 0;

	for (RefList< Uniform >::iterator i = uniformNodes.begin(); i != uniformNodes.end(); ++i)
	{
		CGparameter parameter = cellGcmCgGetNamedParameter(program, (*i)->getParameterName().c_str());
		if (!parameter)
			continue;

		outParameterMap[(*i)->getParameterName()].parameters.push_back(parameter);
		outParameterMap[(*i)->getParameterName()].sampler = false;
		outParameterMap[(*i)->getParameterName()].offset = offset;

		switch ((*i)->getParameterType())
		{
		case PtScalar:
			outParameterMap[(*i)->getParameterName()].stride = 1;
			offset += 1;
			break;

		case PtVector:
			outParameterMap[(*i)->getParameterName()].stride = 4;
			offset += 4;
			break;

		case PtMatrix:
			outParameterMap[(*i)->getParameterName()].stride = 4 * 4;
			offset += 4 * 4;
			break;
		}
	}

	for (RefList< IndexedUniform >::iterator i = indexedUniformNodes.begin(); i != indexedUniformNodes.end(); ++i)
	{
		for (int j = 0; j < (*i)->getLength(); ++j)
		{
			std::wstring parameterName = (*i)->getParameterName() + "[" + toString(j) + "]";

			CGparameter parameter = cellGcmCgGetNamedParameter(program, parameterName.c_str());
			if (!parameter)
				break;

			outParameterMap[(*i)->getParameterName()].parameters.push_back(parameter);

			if (j == 0)
			{
				outParameterMap[(*i)->getParameterName()].sampler = false;
				outParameterMap[(*i)->getParameterName()].offset = offset;
			}

			switch ((*i)->getParameterType())
			{
			case PtScalar:
				outParameterMap[(*i)->getParameterName()].stride = 1;
				offset += 1;
				break;

			case PtVector:
				outParameterMap[(*i)->getParameterName()].stride = 4;
				offset += 4;
				break;

			case PtMatrix:
				outParameterMap[(*i)->getParameterName()].stride = 4 * 4;
				offset += 4 * 4;
				break;
			}
		}
	}	

	for (RefList< Sampler >::iterator i = samplerNodes.begin(); i != samplerNodes.end(); ++i)
	{
		CGparameter parameter = cellGcmCgGetNamedParameter(program, ("SamplerTexture_" + (*i)->getParameterName()).c_str());
		if (!parameter)
			continue;

		CGresource resource = cellGcmCgGetParameterResource(program, parameter);
		if (resource < CG_TEXUNIT0 || resource > CG_TEXUNIT15)
			continue;

		outParameterMap[(*i)->getParameterName()].parameters.push_back(parameter);
		outParameterMap[(*i)->getParameterName()].sampler = true;
		outParameterMap[(*i)->getParameterName()].stage = resource - CG_TEXUNIT0;
	}

	return offset;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderPs3", ShaderPs3, Shader)

ShaderPs3::ShaderPs3()
:	m_vertexShaderImage(0)
,	m_pixelShaderImage(0)
,	m_pixelShaderZOnlyImage(0)
,	m_vertexShaderUCodeSize(0)
,	m_vertexShaderUCode(0)
,	m_pixelShaderUCodeSize(0)
,	m_pixelShaderUCode(0)
,	m_pixelShaderOffset(0)
,	m_pixelShaderZOnlyUCodeSize(0)
,	m_pixelShaderZOnlyUCode(0)
,	m_pixelShaderZOnlyOffset(0)
{
	memset(m_pixelTextures, 0, sizeof(m_pixelTextures));
}

ShaderPs3::~ShaderPs3()
{
	destroy();
}

bool ShaderPs3::create(
	const ShaderGraph* shaderGraph,
	const std::wstring& vertexShader,
	const std::wstring& pixelShader,
	const RenderState& renderState
)
{
	int32_t ret;
	int size;

	ret = compile_program_from_string(
		vertexShader.c_str(),
		"sce_vp_rsx",
		"main",
		NULL,
		&m_vertexShaderImage
	);
	if (ret != 0 || !m_vertexShaderImage)
	{
		log::error << L"Create shader failed, compile error in vertex shader" << Endl;
		log::error << vertexShader << Endl;
		return false;
	}

	ret = compile_program_from_string(
		pixelShader.c_str(),
		"sce_fp_rsx",
		"main",
		NULL,
		&m_pixelShaderImage
	);
	if (ret != 0 || !m_pixelShaderImage)
	{
		log::error << L"Create shader failed, compile error in pixel shader" << Endl;
		log::error << pixelShader << Endl;
		return false;
	}

	ret = compile_program_from_string(
		pixelShader.c_str(),
		"sce_fp_rsx",
		"main_zonly",
		NULL,
		&m_pixelShaderZOnlyImage
	);
	if (ret != 0 || !m_pixelShaderZOnlyImage)
	{
		log::error << L"Create shader failed, compile error in depth pixel shader" << Endl;
		log::error << pixelShader << Endl;
		return false;
	}

	cellGcmCgInitProgram((CGprogram)m_vertexShaderImage);
	cellGcmCgInitProgram((CGprogram)m_pixelShaderImage);
	cellGcmCgInitProgram((CGprogram)m_pixelShaderZOnlyImage);

	cellGcmCgGetUCode((CGprogram)m_vertexShaderImage, &m_vertexShaderUCode, &m_vertexShaderUCodeSize);
	cellGcmCgGetUCode((CGprogram)m_pixelShaderImage, &m_pixelShaderUCode, &m_pixelShaderUCodeSize);
	cellGcmCgGetUCode((CGprogram)m_pixelShaderZOnlyImage, &m_pixelShaderZOnlyUCode, &m_pixelShaderZOnlyUCodeSize);

	void* aligned = LocalMemoryAllocator::getInstance().allocAlign(m_pixelShaderUCodeSize, 64);
	memcpy(aligned, m_pixelShaderUCode, m_pixelShaderUCodeSize);
	m_pixelShaderUCode = aligned;

	ret = cellGcmAddressToOffset(m_pixelShaderUCode, &m_pixelShaderOffset);
	if (ret != CELL_OK)
	{
		log::error << L"Create shader failed, unable to get offset to pixel shader" << Endl;
		return false;
	}

	aligned = LocalMemoryAllocator::getInstance().allocAlign(m_pixelShaderZOnlyUCodeSize, 64);
	memcpy(aligned, m_pixelShaderZOnlyUCode, m_pixelShaderZOnlyUCodeSize);
	m_pixelShaderZOnlyUCode = aligned;

	ret = cellGcmAddressToOffset(m_pixelShaderZOnlyUCode, &m_pixelShaderZOnlyOffset);
	if (ret != CELL_OK)
	{
		log::error << L"Create shader failed, unable to get offset to pixel shader" << Endl;
		return false;
	}

	size = buildParameterMap(shaderGraph, (CGprogram)m_vertexShaderImage, m_vertexParameterMap);
	m_vertexParameters.resize(size);

	size = buildParameterMap(shaderGraph, (CGprogram)m_pixelShaderImage, m_pixelParameterMap);
	m_pixelParameters.resize(size);

	m_renderState = renderState;

	return true;
}

void ShaderPs3::destroy()
{
	if (m_pixelShaderUCode)
	{
		LocalMemoryAllocator::getInstance().free(m_pixelShaderUCode);

		m_pixelShaderUCode = 0;
		m_pixelShaderOffset = 0;
	}
	if (m_pixelShaderZOnlyUCode)
	{
		LocalMemoryAllocator::getInstance().free(m_pixelShaderZOnlyUCode);

		m_pixelShaderZOnlyUCode = 0;
		m_pixelShaderZOnlyOffset = 0;
	}
}

void ShaderPs3::setFloatParameter(const std::wstring& name, float param)
{
	setFloatArrayParameter(name, &param, 1);
}

void ShaderPs3::setFloatArrayParameter(const std::wstring& name, const float* param, int length)
{
	std::map< std::wstring, Parameter >::iterator i = m_vertexParameterMap.find(name);
	if (i != m_vertexParameterMap.end())
	{
		assert (!i->second.sampler);
		std::memcpy(&m_vertexParameters[i->second.offset], param, length * sizeof(float));
	}

	std::map< std::wstring, Parameter >::iterator j = m_pixelParameterMap.find(name);
	if (j != m_pixelParameterMap.end())
	{
		assert (!j->second.sampler);
		std::memcpy(&m_pixelParameters[j->second.offset], param, length * sizeof(float));
	}
}

void ShaderPs3::setVectorParameter(const std::wstring& name, const Vector4& param)
{
	setFloatArrayParameter(name, (const float*)&param, 4);
}

void ShaderPs3::setVectorArrayParameter(const std::wstring& name, const Vector4* param, int length)
{
	setFloatArrayParameter(name, (const float*)param, 4 * length);
}

void ShaderPs3::setMatrixParameter(const std::wstring& name, const Matrix44& param)
{
	setFloatArrayParameter(name, (const float*)&param, 4 * 4);
}

void ShaderPs3::setMatrixArrayParameter(const std::wstring& name, const Matrix44* param, int length)
{
	setFloatArrayParameter(name, (const float*)param, 4 * 4 * length);
}

void ShaderPs3::setSamplerTexture(const std::wstring& name, Texture* texture)
{
	std::map< std::wstring, Parameter >::iterator i = m_pixelParameterMap.find(name);
	if (i != m_pixelParameterMap.end())
	{
		assert (i->second.sampler);
		m_pixelTextures[i->second.stage] = texture;
	}
}

void ShaderPs3::bind(RenderMode renderMode)
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

	cellGcmSetVertexProgram((CGprogram)m_vertexShaderImage, m_vertexShaderUCode);

	for (std::map< std::wstring, Parameter >::iterator i = m_vertexParameterMap.begin(); i != m_vertexParameterMap.end(); ++i)
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

	if (renderMode == RmDefault)
	{
		cellGcmSetFragmentProgram((CGprogram)m_pixelShaderImage, m_pixelShaderOffset);

		for (std::map< std::wstring, Parameter >::iterator i = m_pixelParameterMap.begin(); i != m_pixelParameterMap.end(); ++i)
		{
			if (i->second.sampler)
				continue;

			for (uint32_t j = 0; j < i->second.parameters.size(); ++j)
			{
				cellGcmSetFragmentProgramParameter(
					gCellGcmCurrentContext,
					(CGprogram)m_pixelShaderImage,
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
	else	// RmZOnly
	{
		cellGcmSetFragmentProgram((CGprogram)m_pixelShaderZOnlyImage, m_pixelShaderZOnlyOffset);
	}
}

	}
}
