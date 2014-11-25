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
	size_t i = 0;

	count >>= 2;
	for (i = 0; i < count; ++i)
	{
		vec_float4 vs = vec_ld(0, ptr2);
		vec_float4 vd = vec_ld(0, ptr1);
		
		if (vec_all_eq(vs, vd) == 0)
		{
			equal = false;
			break;
		}

		ptr1 += 4;
		ptr2 += 4;
	}

	if (equal)
		return true;

	for (; i < count; ++i)
	{
		vec_float4 vs = vec_ld(0, ptr2);
		vec_st(vs, 0, ptr1);
		ptr1 += 4;
		ptr2 += 4;
	}

	return false;
}

		}

StateCachePs3::StateCachePs3()
:	m_inFp32Mode(false)
,	m_vertexUCode(0)
,	m_fragmentOffset(0)
{
	const uint32_t c_invalidFloat = 0;

	m_vertexConstantsShadow = (float*)Alloc::acquireAlign(VertexConstantCount * 4 * sizeof(float), 16, T_FILE_LINE);
	std::memset(m_vertexConstantsShadow, c_invalidFloat, VertexConstantCount * 4 * sizeof(float));

	std::memset(m_vertexAttributes, 0, sizeof(m_vertexAttributes));
	std::memset(m_textures, 0, sizeof(m_textures));
	std::memset(m_textureOffsets, 0, sizeof(m_textureOffsets));
	std::memset(m_textureLods, 0, sizeof(m_textureLods));
	std::memset(m_textureAnisotropy, 0, sizeof(m_textureAnisotropy));
}

StateCachePs3::~StateCachePs3()
{
	Alloc::freeAlign(m_vertexConstantsShadow);
}

void StateCachePs3::setInFp32Mode(bool inFp32Mode)
{
	m_inFp32Mode = inFp32Mode;
}

void StateCachePs3::setRenderState(const RenderStateGCM& rs)
{
	if (rs.cullFaceEnable != m_renderState.cullFaceEnable)
	{
		T_GCM_CALL(cellGcmSetCullFaceEnable)(gCellGcmCurrentContext, rs.cullFaceEnable);
		m_renderState.cullFaceEnable = rs.cullFaceEnable;
	}

	if (rs.cullFaceEnable != CELL_GCM_FALSE && (rs.cullFace != m_renderState.cullFace))
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

	if (rs.depthTestEnable != CELL_GCM_FALSE && (rs.depthFunc != m_renderState.depthFunc))
	{
		// Reverse depth function if we use a "1-Z" depth buffer.
		uint32_t depthFunc = rs.depthFunc;
		switch (rs.depthFunc)
		{
		case CELL_GCM_NEVER:
			depthFunc = CELL_GCM_NEVER;
			break;
		case CELL_GCM_LESS:
			depthFunc = CELL_GCM_GREATER;
			break;
		case CELL_GCM_EQUAL:
			depthFunc = CELL_GCM_EQUAL;
			break;
		case CELL_GCM_LEQUAL:
			depthFunc = CELL_GCM_GEQUAL;
			break;
		case CELL_GCM_GREATER:
			depthFunc = CELL_GCM_LESS;
			break;
		case CELL_GCM_NOTEQUAL:
			depthFunc = CELL_GCM_NOTEQUAL;
			break;
		case CELL_GCM_GEQUAL:
			depthFunc = CELL_GCM_LEQUAL;
			break;
		case CELL_GCM_ALWAYS:
			depthFunc = CELL_GCM_ALWAYS;
			break;
		}
		T_GCM_CALL(cellGcmSetDepthFunc)(gCellGcmCurrentContext, depthFunc);
		m_renderState.depthFunc = rs.depthFunc;
	}

	if (rs.stencilTestEnable != m_renderState.stencilTestEnable)
	{
		T_GCM_CALL(cellGcmSetStencilTestEnable)(gCellGcmCurrentContext, rs.stencilTestEnable);
		m_renderState.stencilTestEnable = rs.stencilTestEnable;
	}

	if (
		rs.stencilTestEnable != CELL_GCM_FALSE &&
		(
			rs.stencilFunc != m_renderState.stencilFunc ||
			rs.stencilRef != m_renderState.stencilRef
		)
	)
	{
		T_GCM_CALL(cellGcmSetStencilFunc)(gCellGcmCurrentContext, rs.stencilFunc, rs.stencilRef, ~0UL);
		m_renderState.stencilFunc = rs.stencilFunc;
		m_renderState.stencilRef = rs.stencilRef;
	}

	if (
		rs.stencilTestEnable != CELL_GCM_FALSE &&
		(
			rs.stencilOpFail != m_renderState.stencilOpFail ||
			rs.stencilOpZFail != m_renderState.stencilOpZFail ||
			rs.stencilOpZPass != m_renderState.stencilOpZPass
		)
	)
	{
		T_GCM_CALL(cellGcmSetStencilOp)(gCellGcmCurrentContext, rs.stencilOpFail, rs.stencilOpZFail, rs.stencilOpZPass);
		m_renderState.stencilOpFail = rs.stencilOpFail;
		m_renderState.stencilOpZFail = rs.stencilOpZFail;
		m_renderState.stencilOpZPass = rs.stencilOpZPass;
	}

	if (!m_inFp32Mode)
	{
		if (rs.blendEnable != m_renderState.blendEnable)
		{
			T_GCM_CALL(cellGcmSetBlendEnable)(gCellGcmCurrentContext, rs.blendEnable);
			m_renderState.blendEnable = rs.blendEnable;
		}

		if (rs.blendEnable != CELL_GCM_FALSE && (rs.blendEquation != m_renderState.blendEquation))
		{
			T_GCM_CALL(cellGcmSetBlendEquation)(gCellGcmCurrentContext, rs.blendEquation, rs.blendEquation);
			m_renderState.blendEquation = rs.blendEquation;
		}

		if (rs.blendEnable != CELL_GCM_FALSE && (rs.blendFuncSrc != m_renderState.blendFuncSrc || rs.blendFuncDest != m_renderState.blendFuncDest))
		{
			T_GCM_CALL(cellGcmSetBlendFunc)(gCellGcmCurrentContext, rs.blendFuncSrc, rs.blendFuncDest, rs.blendFuncSrc, rs.blendFuncDest);
			m_renderState.blendFuncSrc = rs.blendFuncSrc;
			m_renderState.blendFuncDest = rs.blendFuncDest;
		}

		if (rs.alphaTestEnable != m_renderState.alphaTestEnable)
		{
			T_GCM_CALL(cellGcmSetAlphaTestEnable)(gCellGcmCurrentContext, rs.alphaTestEnable);
			m_renderState.alphaTestEnable = rs.alphaTestEnable;
		}

		if (rs.alphaTestEnable != CELL_GCM_FALSE && (rs.alphaFunc != m_renderState.alphaFunc || m_renderState.alphaRef != m_renderState.alphaRef))
		{
			T_GCM_CALL(cellGcmSetAlphaFunc)(gCellGcmCurrentContext, rs.alphaFunc, rs.alphaRef);
			m_renderState.alphaFunc = rs.alphaFunc;
			m_renderState.alphaRef = rs.alphaRef;
		}
	}
}

void StateCachePs3::setSamplerState(int32_t stage, const SamplerStateGCM& ss)
{
	SamplerStateGCM& samplerState = m_samplerStates[stage];

	if (
		ss.minFilter != samplerState.minFilter ||
		ss.magFilter != samplerState.magFilter
	)
	{
		T_GCM_CALL(cellGcmSetTextureFilter)(
			gCellGcmCurrentContext,
			stage,
			0,
			ss.minFilter,
			ss.magFilter,
			CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX
		);
		samplerState.minFilter = ss.minFilter;
		samplerState.magFilter = ss.magFilter;
	}

	if (
		ss.wrapU != samplerState.wrapU ||
		ss.wrapV != samplerState.wrapV ||
		ss.wrapW != samplerState.wrapW
	)
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

void StateCachePs3::setSamplerTexture(int32_t stage, const CellGcmTexture* texture, uint16_t maxLod, uint8_t anisotropy)
{
	if (
		maxLod != m_textureLods[stage] ||
		anisotropy != m_textureAnisotropy[stage]
	)
	{
		T_GCM_CALL(cellGcmSetTextureControl)(
			gCellGcmCurrentContext,
			stage,
			CELL_GCM_TRUE,
			0,
			maxLod,
			anisotropy
		);
		m_textureLods[stage] = maxLod;
		m_textureAnisotropy[stage] = anisotropy;
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

void StateCachePs3::setProgram(
	const CGprogram vertexProgram,
	const void* vertexUCode,
	const CGprogram fragmentProgram,
	const uint32_t fragmentOffset,
	bool updateFragmentConstants,
	bool updateFragmentTextures
)
{
	if (vertexUCode != m_vertexUCode)
	{
		T_GCM_CALL(cellGcmSetVertexProgram)(gCellGcmCurrentContext, vertexProgram, vertexUCode);
		m_vertexUCode = vertexUCode;
	}
	if (fragmentOffset != m_fragmentOffset || updateFragmentTextures)
	{
		T_GCM_CALL(cellGcmSetFragmentProgram)(gCellGcmCurrentContext, fragmentProgram, fragmentOffset);
		m_fragmentOffset = fragmentOffset;
	}
	else if (updateFragmentConstants)
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

void StateCachePs3::setViewport(const Viewport& viewport)
{
	float scale[4];
	float offset[4];

	scale[0] = viewport.width * 0.5f;
	scale[1] = viewport.height * -0.5f;
	scale[2] = -(viewport.farZ - viewport.nearZ) * 0.5f;	// Scale negative if we use a "1-Z" depth buffer.
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
}

void StateCachePs3::setVertexDataArray(uint8_t index, uint8_t stride, uint8_t size, uint8_t type, uint8_t location, uint32_t offset)
{
	VertexAttribute& vattr = m_vertexAttributes[index];
	if (
		vattr.stride != stride ||
		vattr.size != size ||
		vattr.type != type ||
		vattr.location != location ||
		vattr.offset != offset
	)
	{
		T_GCM_CALL(cellGcmSetVertexDataArray)(
			gCellGcmCurrentContext,
			index,
			0,
			stride,
			size,
			type,
			location,
			offset
		);

		vattr.stride = stride;
		vattr.size = size;
		vattr.type = type;
		vattr.location = location;
		vattr.offset = offset;
	}
}

void StateCachePs3::reset()
{
	m_renderState = RenderStateGCM();
	for (int i = 0; i < SamplerCount; ++i)
	{
		m_samplerStates[i] = SamplerStateGCM();
		m_textureOffsets[i] = 0;
		m_textureLods[i] = 12 << 8;
		m_textureAnisotropy[i] = CELL_GCM_TEXTURE_MAX_ANISO_1;
	}
	for (int i = 0; i < sizeof_array(m_vertexAttributes); ++i)
	{
		m_vertexAttributes[i].stride = 0;
		m_vertexAttributes[i].size = 0;
		m_vertexAttributes[i].type = CELL_GCM_VERTEX_F;
		m_vertexAttributes[i].location = CELL_GCM_LOCATION_LOCAL;
		m_vertexAttributes[i].offset = 0;
	}
}

	}
}
