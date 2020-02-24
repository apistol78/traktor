#pragma once

#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Render
 */
struct RenderGraphTargetDesc
{
	TextureFormat colorFormat;

	RenderGraphTargetDesc()
	:	colorFormat(TfInvalid)
	{
	}
};

/*!
 * \ingroup Render
 */
struct RenderGraphTargetSetDesc
{
	enum { MaxColorTargets = RenderTargetSetCreateDesc::MaxTargets };

	int32_t count;
	int32_t width;
	int32_t height;
	int32_t referenceWidthDenom;
	int32_t referenceHeightDenom;
	int32_t maxWidth;
	int32_t maxHeight;
	bool createDepthStencil;
	bool usingPrimaryDepthStencil;
	bool usingDepthStencilAsTexture;
	bool ignoreStencil;
	bool generateMips;
	RenderGraphTargetDesc targets[MaxColorTargets];

	RenderGraphTargetSetDesc()
	:	count(0)
	,	width(0)
	,	height(0)
	,	referenceWidthDenom(0)
	,	referenceHeightDenom(0)
	,	maxWidth(0)
	,	maxHeight(0)
	,	createDepthStencil(false)
	,	usingPrimaryDepthStencil(false)
	,	usingDepthStencilAsTexture(false)
	,	ignoreStencil(false)
	,	generateMips(false)
	{
	}
};

	}
}
