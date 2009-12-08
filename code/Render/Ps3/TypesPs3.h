#ifndef traktor_render_TypesPs3_H
#define traktor_render_TypesPs3_H

#include "Render/Ps3/PlatformPs3.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

struct SamplerState
{
	uint8_t minFilter;
	uint8_t magFilter;
	uint8_t wrapU;
	uint8_t wrapV;
	uint8_t wrapW;

	SamplerState()
	:	minFilter(CELL_GCM_TEXTURE_NEAREST_LINEAR)
	,	magFilter(CELL_GCM_TEXTURE_LINEAR)
	,	wrapU(CELL_GCM_TEXTURE_WRAP)
	,	wrapV(CELL_GCM_TEXTURE_WRAP)
	,	wrapW(CELL_GCM_TEXTURE_CLAMP_TO_EDGE)
	{
	}
};

struct RenderState
{
	uint32_t cullFaceEnable;
	uint32_t cullFace;
	uint32_t blendEnable;
	uint16_t blendEquation;
	uint16_t blendFuncSrc;
	uint16_t blendFuncDest;
	uint32_t depthTestEnable;
	uint32_t colorMask;
	uint32_t depthMask;
	uint32_t depthFunc;
	uint32_t alphaTestEnable;
	uint32_t alphaFunc;
	uint32_t alphaRef;
	SamplerState samplerStates[8];

	RenderState()
	:	cullFaceEnable(CELL_GCM_TRUE)
	,	cullFace(CELL_GCM_BACK)
	,	blendEnable(CELL_GCM_FALSE)
	,	blendEquation(CELL_GCM_FUNC_ADD)
	,	blendFuncSrc(CELL_GCM_ONE)
	,	blendFuncDest(CELL_GCM_ZERO)
	,	depthTestEnable(CELL_GCM_TRUE)
	,	colorMask(CELL_GCM_COLOR_MASK_B | CELL_GCM_COLOR_MASK_G | CELL_GCM_COLOR_MASK_R | CELL_GCM_COLOR_MASK_A)
	,	depthMask(CELL_GCM_TRUE)
	,	depthFunc(CELL_GCM_LEQUAL)
	,	alphaTestEnable(CELL_GCM_FALSE)
	,	alphaFunc(CELL_GCM_ALWAYS)
	,	alphaRef(0)
	{
	}
};

bool getGcmTextureInfo(TextureFormat textureFormat, int& outByteSize, uint8_t& outGcmFormat);

void cellUtilConvertLinearToSwizzle(
	uint8_t *dst,
	const uint8_t *src,
	const uint32_t width,
	const uint32_t height,
	const uint32_t depth
);

	}
}

#endif	// traktor_render_TypesPs3_H
