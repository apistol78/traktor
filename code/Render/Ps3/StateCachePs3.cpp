#include <vec_types.h>
#include "Core/Memory/Alloc.h"
#include "Render/Ps3/StateCachePs3.h"
#include "Render/Ps3/RenderTargetPs3.h"
#include "Render/Ps3/SimpleTexturePs3.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

bool compareExchangeEqual4(float* ptr1, const float* ptr2, size_t count)
{
	T_ASSERT ((count & 3) == 0);

	const float* src = ptr2;
	float* dst = ptr1;
	bool equal = true;

	for (size_t i = 0; i < (count >> 2); ++i)
	{
		if (
			dst[0] != src[0] ||
			dst[1] != src[1] ||
			dst[2] != src[2] ||
			dst[3] != src[3]
		)
		{
			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = src[2];
			dst[3] = src[3];
			equal = false;
		}

		src += 4;
		dst += 4;
	}

	return equal;
}

		}

StateCachePs3::StateCachePs3()
:	m_inFp32Mode(false)
,	m_vertexUCode(0)
,	m_fragmentOffset(0)
{
	m_vertexConstantsShadow = (float*)Alloc::acquireAlign(VertexConstantCount * 4 * sizeof(float), 16);
	std::memset(m_vertexConstantsShadow, 0, VertexConstantCount * 4 * sizeof(float));

	T_GCM_CALL(cellGcmSetVertexProgramParameterBlock)(
		gCellGcmCurrentContext,
		0,
		VertexConstantCount,
		m_vertexConstantsShadow
	);
}

StateCachePs3::~StateCachePs3()
{
	Alloc::freeAlign(m_vertexConstantsShadow);
}

void StateCachePs3::setInFp32Mode(bool inFp32Mode)
{
	m_inFp32Mode = inFp32Mode;
}

void StateCachePs3::setRenderState(const RenderState& rs)
{
	if (rs.cullFaceEnable != m_renderState.cullFaceEnable)
	{
		T_GCM_CALL(cellGcmSetCullFaceEnable)(gCellGcmCurrentContext, rs.cullFaceEnable);
		m_renderState.cullFaceEnable = rs.cullFaceEnable;
	}

	if (/*rs.cullFaceEnable && */(rs.cullFace != m_renderState.cullFace))
	{
		T_GCM_CALL(cellGcmSetCullFace)(gCellGcmCurrentContext, rs.cullFace);
		m_renderState.cullFace = rs.cullFace;
	}

	if (rs.depthTestEnable != m_renderState.depthTestEnable)
	{
		T_GCM_CALL(cellGcmSetDepthTestEnable)(gCellGcmCurrentContext, rs.depthTestEnable);
		m_renderState.depthTestEnable = rs.depthTestEnable;
	}

	if (rs.colorMask != m_renderState.colorMask)
	{
		T_GCM_CALL(cellGcmSetColorMask)(gCellGcmCurrentContext, rs.colorMask);
		m_renderState.colorMask = rs.colorMask;
	}

	if (rs.depthMask != m_renderState.depthMask)
	{
		T_GCM_CALL(cellGcmSetDepthMask)(gCellGcmCurrentContext, rs.depthMask);
		m_renderState.depthMask = rs.depthMask;
	}

	if (/*rs.depthTestEnable && */(rs.depthFunc != m_renderState.depthFunc))
	{
		T_GCM_CALL(cellGcmSetDepthFunc)(gCellGcmCurrentContext, rs.depthFunc);
		m_renderState.depthFunc = rs.depthFunc;
	}

	if (!m_inFp32Mode)
	{
		if (rs.blendEnable != m_renderState.blendEnable)
		{
			T_GCM_CALL(cellGcmSetBlendEnable)(gCellGcmCurrentContext, rs.blendEnable);
			m_renderState.blendEnable = rs.blendEnable;
		}

		if (/*rs.blendEnable && */(rs.blendEquation != m_renderState.blendEquation))
		{
			T_GCM_CALL(cellGcmSetBlendEquation)(gCellGcmCurrentContext, rs.blendEquation, CELL_GCM_FUNC_ADD);
			m_renderState.blendEquation = rs.blendEquation;
		}

		if (/*rs.blendEnable && */(rs.blendFuncSrc != m_renderState.blendFuncSrc || rs.blendFuncDest != m_renderState.blendFuncDest))
		{
			T_GCM_CALL(cellGcmSetBlendFunc)(gCellGcmCurrentContext, rs.blendFuncSrc, rs.blendFuncDest, CELL_GCM_ONE, CELL_GCM_ZERO);
			m_renderState.blendFuncSrc = rs.blendFuncSrc;
			m_renderState.blendFuncDest = rs.blendFuncDest;
		}

		if (rs.alphaTestEnable != m_renderState.alphaTestEnable)
		{
			T_GCM_CALL(cellGcmSetAlphaTestEnable)(gCellGcmCurrentContext, rs.alphaTestEnable);
			m_renderState.alphaTestEnable = rs.alphaTestEnable;
		}

		if (/*rs.alphaTestEnable && */(rs.alphaFunc != m_renderState.alphaFunc || m_renderState.alphaRef != m_renderState.alphaRef))
		{
			T_GCM_CALL(cellGcmSetAlphaFunc)(gCellGcmCurrentContext, rs.alphaFunc, rs.alphaRef);
			m_renderState.alphaFunc = rs.alphaFunc;
			m_renderState.alphaRef = rs.alphaRef;
		}
	}
}

void StateCachePs3::setProgram(const CGprogram vertexProgram, const void* vertexUCode, const CGprogram fragmentProgram, const uint32_t fragmentOffset, bool updateFragmentProgram)
{
	if (vertexUCode != m_vertexUCode)
	{
		T_GCM_CALL(cellGcmSetVertexProgram)(gCellGcmCurrentContext, vertexProgram, vertexUCode);
		m_vertexUCode = vertexUCode;
	}
	if (fragmentOffset != m_fragmentOffset)
	{
		T_GCM_CALL(cellGcmSetFragmentProgram)(gCellGcmCurrentContext, fragmentProgram, fragmentOffset);
		m_fragmentOffset = fragmentOffset;
	}
	else if (updateFragmentProgram)
	{
		T_GCM_CALL(cellGcmSetUpdateFragmentProgramParameter)(gCellGcmCurrentContext, fragmentOffset);
	}
}

void StateCachePs3::setVertexShaderConstant(uint32_t registerOffset, uint32_t registerCount, const float* constantData)
{
	float* shadow = &m_vertexConstantsShadow[registerOffset * 4];
	if (!compareExchangeEqual4(shadow, constantData, registerCount * 4))
	{
		T_GCM_CALL(cellGcmSetVertexProgramParameterBlock)(
			gCellGcmCurrentContext,
			registerOffset,
			registerCount,
			constantData
		);
	}
}

void StateCachePs3::setTexture(uint16_t stage, ITexture* texture, const SamplerState& samplerState)
{
	if (texture != m_textures[stage])
	{
		if (texture)
		{
			if (is_a< SimpleTexturePs3 >(texture))
				static_cast< SimpleTexturePs3* >(texture)->bind(stage, samplerState);
			else if (is_a< RenderTargetPs3 >(texture))
				static_cast< RenderTargetPs3* >(texture)->bind(stage, samplerState);
		}
		m_textures[stage] = texture;
	}
}

void StateCachePs3::reset(bool force)
{
	if (!force)
	{
		setRenderState(RenderState());
	}
	else
	{
		m_renderState = RenderState();

		T_GCM_CALL(cellGcmSetCullFaceEnable)(gCellGcmCurrentContext, m_renderState.cullFaceEnable);
		T_GCM_CALL(cellGcmSetCullFace)(gCellGcmCurrentContext, m_renderState.cullFace);
		T_GCM_CALL(cellGcmSetDepthTestEnable)(gCellGcmCurrentContext, m_renderState.depthTestEnable);
		T_GCM_CALL(cellGcmSetColorMask)(gCellGcmCurrentContext, m_renderState.colorMask);
		T_GCM_CALL(cellGcmSetDepthMask)(gCellGcmCurrentContext, m_renderState.depthMask);
		T_GCM_CALL(cellGcmSetDepthFunc)(gCellGcmCurrentContext, m_renderState.depthFunc);

		if (!m_inFp32Mode)
		{
			T_GCM_CALL(cellGcmSetBlendEnable)(gCellGcmCurrentContext, m_renderState.blendEnable);
			T_GCM_CALL(cellGcmSetBlendEquation)(gCellGcmCurrentContext, m_renderState.blendEquation, CELL_GCM_FUNC_ADD);
			T_GCM_CALL(cellGcmSetBlendFunc)(gCellGcmCurrentContext, m_renderState.blendFuncSrc, m_renderState.blendFuncDest, CELL_GCM_ONE, CELL_GCM_ZERO);
			T_GCM_CALL(cellGcmSetAlphaTestEnable)(gCellGcmCurrentContext, m_renderState.alphaTestEnable);
			T_GCM_CALL(cellGcmSetAlphaFunc)(gCellGcmCurrentContext, m_renderState.alphaFunc, m_renderState.alphaRef);
		}
	}

	m_vertexUCode = 0;
	m_fragmentOffset = 0;

	resetTextures();
}

void StateCachePs3::resetTextures()
{
	for (int i = 0; i < sizeof_array(m_textures); ++i)
		m_textures[i] = 0;
}

	}
}
