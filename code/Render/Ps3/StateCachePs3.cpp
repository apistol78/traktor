#include <vec_types.h>
#include "Core/Memory/Alloc.h"
#include "Core/Misc/Align.h"
#include "Render/Ps3/StateCachePs3.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

bool compareExchangeEqual4(float* ptr1, const float* ptr2, size_t count)
{
	T_ASSERT ((count & 3) == 0);
	T_ASSERT (alignUp(ptr1, 16) == ptr1);
	T_ASSERT (alignUp(ptr2, 16) == ptr2);

	bool equal = true;
	for (size_t i = 0; i < (count >> 2); ++i)
	{
		vec_float4 vs = vec_ld(0, ptr2);
		vec_float4 vd = vec_ld(0, ptr1);
		
		if (vec_all_eq(vs, vd) == 0)
		{
			vec_st(vs, 0, ptr1);
			equal = false;
		}

		ptr1 += 4;
		ptr2 += 4;
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

	reset(RfRenderState | RfSamplerStates | RfForced);
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

	if (/*rs.cullFaceEnable == CELL_GCM_TRUE && */(rs.cullFace != m_renderState.cullFace))
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

	if (/*rs.depthTestEnable == CELL_GCM_TRUE && */(rs.depthFunc != m_renderState.depthFunc))
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

		if (/*rs.blendEnable == CELL_GCM_TRUE && */(rs.blendEquation != m_renderState.blendEquation))
		{
			T_GCM_CALL(cellGcmSetBlendEquation)(gCellGcmCurrentContext, rs.blendEquation, CELL_GCM_FUNC_ADD);
			m_renderState.blendEquation = rs.blendEquation;
		}

		if (/*rs.blendEnable == CELL_GCM_TRUE && */(rs.blendFuncSrc != m_renderState.blendFuncSrc || rs.blendFuncDest != m_renderState.blendFuncDest))
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

		if (/*rs.alphaTestEnable == CELL_GCM_TRUE && */(rs.alphaFunc != m_renderState.alphaFunc || m_renderState.alphaRef != m_renderState.alphaRef))
		{
			T_GCM_CALL(cellGcmSetAlphaFunc)(gCellGcmCurrentContext, rs.alphaFunc, rs.alphaRef);
			m_renderState.alphaFunc = rs.alphaFunc;
			m_renderState.alphaRef = rs.alphaRef;
		}
	}
}

void StateCachePs3::setSamplerState(int32_t stage, const SamplerState& ss)
{
	SamplerState& samplerState = m_samplerStates[stage];

	if (ss.minFilter != samplerState.minFilter || ss.magFilter != samplerState.magFilter)
	{
		T_GCM_CALL(cellGcmSetTextureFilter)(
			gCellGcmCurrentContext,
			stage,
			(-0x0080) & (0x1fff),		// -0.50 mip bias
			ss.minFilter,
			ss.magFilter,
			CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX
		);
		samplerState.minFilter = ss.minFilter;
		samplerState.magFilter = ss.magFilter;
	}

	if (ss.wrapU != samplerState.wrapU || ss.wrapV != samplerState.wrapV || ss.wrapW != samplerState.wrapW)
	{
		T_GCM_CALL(cellGcmSetTextureAddress)(
			gCellGcmCurrentContext,
			stage,
			ss.wrapU,
			ss.wrapV,
			ss.wrapW,
			CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL,
			CELL_GCM_TEXTURE_ZFUNC_NEVER,
			0
		);
		samplerState.wrapU = ss.wrapU;
		samplerState.wrapV = ss.wrapV;
		samplerState.wrapW = ss.wrapW;
	}
}

void StateCachePs3::setSamplerTexture(int32_t stage, const CellGcmTexture* texture, uint16_t maxLod)
{
	if (maxLod != m_textureLods[stage])
	{
		T_GCM_CALL(cellGcmSetTextureControl)(
			gCellGcmCurrentContext,
			stage,
			CELL_GCM_TRUE,
			0,
			maxLod,
			CELL_GCM_TEXTURE_MAX_ANISO_1
		);
		m_textureLods[stage] = maxLod;
	}
	if (texture != m_textures[stage] || texture->offset != m_textureOffsets[stage])
	{
		T_GCM_CALL(cellGcmSetTexture)(
			gCellGcmCurrentContext,
			stage,
			texture
		);
		m_textures[stage] = texture;
		m_textureOffsets[stage] = texture->offset;
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

void StateCachePs3::setColorMask(uint32_t colorMask)
{
	if (colorMask != m_colorMask)
	{
		T_GCM_CALL(cellGcmSetColorMask)(gCellGcmCurrentContext, colorMask);
		m_colorMask = colorMask;
	}
}

void StateCachePs3::setViewport(const Viewport& viewport)
{
	/*if (
		viewport.left != m_viewport.left ||
		viewport.top != m_viewport.top ||
		viewport.width != m_viewport.width ||
		viewport.height != m_viewport.height ||
		viewport.nearZ != m_viewport.nearZ ||
		viewport.farZ != m_viewport.farZ
	)*/
	{
		float scale[4];
		float offset[4];

		scale[0] = viewport.width * 0.5f;
		scale[1] = viewport.height * -0.5f;
		scale[2] = (viewport.farZ - viewport.nearZ) * 0.5f;
		scale[3] = 0.0f;

		offset[0] = viewport.left + scale[0];
		offset[1] = viewport.height - viewport.top + scale[1];
		offset[2] = (viewport.farZ + viewport.nearZ) * 0.5f;
		offset[3] = 0.0f;

		T_GCM_CALL(cellGcmSetViewport)(
			gCellGcmCurrentContext,
			viewport.left,
			viewport.top,
			viewport.width,
			viewport.height,
			viewport.nearZ,
			viewport.farZ,
			scale,
			offset
		);

		T_GCM_CALL(cellGcmSetScissor)(
			gCellGcmCurrentContext,
			viewport.left,
			viewport.top,
			viewport.width,
			viewport.height
		);

		m_viewport = viewport;
	}
}

void StateCachePs3::reset(uint32_t flags)
{
	if (!(flags & RfForced))
	{
		if (flags & RfRenderState)
			setRenderState(RenderState());

		if (flags & RfSamplerStates)
		{
			for (int i = 0; i < SamplerCount; ++i)
			{
				setSamplerState(i, SamplerState());

				if (m_textureLods[i] != uint16_t(~0))
				{
					T_GCM_CALL(cellGcmSetTextureControl)(
						gCellGcmCurrentContext,
						i,
						CELL_GCM_FALSE,
						0,
						0,
						CELL_GCM_TEXTURE_MAX_ANISO_1
					);
				}
			}
		}
	}
	else
	{
		m_renderState = RenderState();

		if (flags & RfRenderState)
		{
			T_GCM_CALL(cellGcmSetCullFaceEnable)(gCellGcmCurrentContext, m_renderState.cullFaceEnable);
			T_GCM_CALL(cellGcmSetCullFace)(gCellGcmCurrentContext, m_renderState.cullFace);
			T_GCM_CALL(cellGcmSetDepthTestEnable)(gCellGcmCurrentContext, m_renderState.depthTestEnable);
			T_GCM_CALL(cellGcmSetColorMask)(gCellGcmCurrentContext, m_renderState.colorMask);
			T_GCM_CALL(cellGcmSetDepthMask)(gCellGcmCurrentContext, m_renderState.depthMask);
			T_GCM_CALL(cellGcmSetDepthFunc)(gCellGcmCurrentContext, m_renderState.depthFunc);

			T_GCM_CALL(cellGcmSetBlendEnable)(gCellGcmCurrentContext, m_renderState.blendEnable);
			T_GCM_CALL(cellGcmSetBlendEquation)(gCellGcmCurrentContext, m_renderState.blendEquation, CELL_GCM_FUNC_ADD);
			T_GCM_CALL(cellGcmSetBlendFunc)(gCellGcmCurrentContext, m_renderState.blendFuncSrc, m_renderState.blendFuncDest, CELL_GCM_ONE, CELL_GCM_ZERO);
			T_GCM_CALL(cellGcmSetAlphaTestEnable)(gCellGcmCurrentContext, m_renderState.alphaTestEnable);
			T_GCM_CALL(cellGcmSetAlphaFunc)(gCellGcmCurrentContext, m_renderState.alphaFunc, m_renderState.alphaRef);
		}

		if (flags & RfSamplerStates)
		{
			for (int i = 0; i < SamplerCount; ++i)
			{
				m_samplerStates[i] = SamplerState();

				T_GCM_CALL(cellGcmSetTextureFilter)(
					gCellGcmCurrentContext,
					i,
					0,
					m_samplerStates[i].minFilter,
					m_samplerStates[i].magFilter,
					CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX
				);

				T_GCM_CALL(cellGcmSetTextureAddress)(
					gCellGcmCurrentContext,
					i,
					m_samplerStates[i].wrapU,
					m_samplerStates[i].wrapV,
					m_samplerStates[i].wrapW,
					CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL,
					CELL_GCM_TEXTURE_ZFUNC_NEVER,
					0
				);

				T_GCM_CALL(cellGcmSetTextureControl)(
					gCellGcmCurrentContext,
					i,
					CELL_GCM_FALSE,
					0,
					0,
					CELL_GCM_TEXTURE_MAX_ANISO_1
				);
			}
		}

		T_GCM_CALL(cellGcmSetColorMask)(gCellGcmCurrentContext, CELL_GCM_COLOR_MASK_R | CELL_GCM_COLOR_MASK_G | CELL_GCM_COLOR_MASK_B | CELL_GCM_COLOR_MASK_A);
	}

	m_vertexUCode = 0;
	m_fragmentOffset = 0;

	if (flags & RfSamplerStates)
	{
		for (int i = 0; i < sizeof_array(m_textures); ++i)
			m_textures[i] = 0;

		for (int i = 0; i < sizeof_array(m_textureOffsets); ++i)
			m_textureOffsets[i] = 0;

		for (int i = 0; i < sizeof_array(m_textureLods); ++i)
			m_textureLods[i] = uint16_t(~0);
	}

	m_colorMask = CELL_GCM_COLOR_MASK_R | CELL_GCM_COLOR_MASK_G | CELL_GCM_COLOR_MASK_B | CELL_GCM_COLOR_MASK_A;
}

	}
}
