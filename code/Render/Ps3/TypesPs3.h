/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_TypesPs3_H
#define traktor_render_TypesPs3_H

#include "Render/Ps3/PlatformPs3.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

struct SamplerStateGCM
{
	uint8_t minFilter;
	uint8_t magFilter;
	uint8_t wrapU;
	uint8_t wrapV;
	uint8_t wrapW;

	SamplerStateGCM()
	:	minFilter(CELL_GCM_TEXTURE_NEAREST_LINEAR)
	,	magFilter(CELL_GCM_TEXTURE_LINEAR)
	,	wrapU(CELL_GCM_TEXTURE_WRAP)
	,	wrapV(CELL_GCM_TEXTURE_WRAP)
	,	wrapW(CELL_GCM_TEXTURE_CLAMP_TO_EDGE)
	{
	}
};

struct RenderStateGCM
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
	uint32_t stencilTestEnable;
	uint32_t stencilFunc;
	uint32_t stencilRef;
	uint32_t stencilOpFail;
	uint32_t stencilOpZFail;
	uint32_t stencilOpZPass;
	SamplerStateGCM samplerStates[8];

	RenderStateGCM()
	:	cullFaceEnable(CELL_GCM_FALSE)
	,	cullFace(CELL_GCM_BACK)
	,	blendEnable(CELL_GCM_FALSE)
	,	blendEquation(CELL_GCM_FUNC_ADD)
	,	blendFuncSrc(CELL_GCM_ONE)
	,	blendFuncDest(CELL_GCM_ZERO)
	,	depthTestEnable(CELL_GCM_FALSE)
	,	colorMask(CELL_GCM_COLOR_MASK_B | CELL_GCM_COLOR_MASK_G | CELL_GCM_COLOR_MASK_R | CELL_GCM_COLOR_MASK_A)
	,	depthMask(CELL_GCM_TRUE)
	,	depthFunc(CELL_GCM_LESS)
	,	alphaTestEnable(CELL_GCM_FALSE)
	,	alphaFunc(CELL_GCM_ALWAYS)
	,	alphaRef(0)
	,	stencilTestEnable(CELL_GCM_FALSE)
	,	stencilFunc(CELL_GCM_ALWAYS)
	,	stencilRef(0)
	,	stencilOpFail(CELL_GCM_KEEP)
	,	stencilOpZFail(CELL_GCM_KEEP)
	,	stencilOpZPass(CELL_GCM_KEEP)
	{
	}
};

struct FragmentOffset
{
	uint32_t ucodeOffset;
	uint32_t parameterOffset;
};

struct ProgramScalar
{
	uint16_t vertexRegisterIndex;
	uint16_t vertexRegisterCount;
	std::vector< FragmentOffset > fragmentOffsets;
	uint16_t offset;
};

struct ProgramSampler
{
	uint16_t stage;
	uint16_t texture;
};

enum ScalarUsage
{
	SuVertex = 1,
	SuPixel = 2
};

struct ScalarParameter
{
	uint32_t offset;
	uint8_t usage;
};

struct ResolutionDesc
{
	int32_t width;
	int32_t height;
	uint32_t refreshRates;
	bool stereoscopic;
	int32_t id;
};

extern const ResolutionDesc c_resolutionDescs[];

const ResolutionDesc* findResolutionDesc(int32_t width, int32_t height, bool stereoscopic);

bool getGcmSurfaceInfo(TextureFormat textureFormat, uint8_t& outGcmTextureFormat, uint8_t& outGcmSurfaceFormat, uint8_t& outByteSize);

bool getGcmTextureInfo(TextureFormat textureFormat, uint8_t& outGcmFormat);

void cellUtilConvertLinearToSwizzle(
	uint8_t *dst,
	const uint8_t *src,
	const uint32_t width,
	const uint32_t height,
	const uint32_t depth
);

std::wstring lookupGcmError(int32_t err);

	}
}

#endif	// traktor_render_TypesPs3_H
