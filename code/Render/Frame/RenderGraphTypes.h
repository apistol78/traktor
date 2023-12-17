/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/Types.h"

namespace traktor::render
{

/*!
 * \ingroup Render
 */
struct RenderGraphTargetDesc
{
	TextureFormat colorFormat = TfInvalid;
};

/*!
 * \ingroup Render
 */
struct RenderGraphTargetSetDesc
{
	enum { MaxColorTargets = RenderTargetSetCreateDesc::MaxTargets };

	int32_t count = 0;
	int32_t width = 0;
	int32_t height = 0;
	int32_t referenceWidthMul = 1;
	int32_t referenceWidthDenom = 0;
	int32_t referenceHeightMul = 1;
	int32_t referenceHeightDenom = 0;
	int32_t maxWidth = 0;
	int32_t maxHeight = 0;
	bool createDepthStencil = false;
	bool usingDepthStencilAsTexture = false;
	bool ignoreStencil = false;
	bool generateMips = false;
	RenderGraphTargetDesc targets[MaxColorTargets];
};

/*!
 */
struct RenderGraphTextureDesc
{
	int32_t width = 0;
	int32_t height = 0;
	int32_t mipCount = 0;
	TextureFormat format = TfInvalid;
};

}
