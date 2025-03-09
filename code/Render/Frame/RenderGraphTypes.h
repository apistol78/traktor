/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

/*!
 * \ingroup Render
 */
struct RenderGraphTargetDesc
{
	TextureFormat colorFormat = TfInvalid;
	bool sRGB = false;
};

/*!
 * \ingroup Render
 */
struct RenderGraphTargetSetDesc
{
	enum
	{
		MaxColorTargets = RenderTargetSetCreateDesc::MaxTargets
	};

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
	int32_t referenceWidthMul = 1;
	int32_t referenceWidthDenom = 0;
	int32_t referenceHeightMul = 1;
	int32_t referenceHeightDenom = 0;
	int32_t mipCount = 0;
	TextureFormat format = TfInvalid;
};

/*!
 */
struct RenderGraphBufferDesc
{
	int32_t elementSize;		   //!< Size in bytes of each element in buffer.
	int32_t elementCount;		   //!< Number of elements in buffer.
	int32_t referenceWidthMul = 1; //!< If set used to calculate number of elements based on size of output render target.
	int32_t referenceWidthDenom = 0;
	int32_t referenceHeightMul = 1;
	int32_t referenceHeightDenom = 0;
};

class T_DLLCLASS RGHandle
{
public:
	handle_t get() const { return m_data; }

protected:
	handle_t m_data = ~0U;

	RGHandle() = default;

	explicit RGHandle(handle_t data)
		: m_data(data)
	{
	}
};

class T_DLLCLASS RGTargetSet : public RGHandle
{
public:
	const static RGTargetSet Invalid;

	const static RGTargetSet Output;

	RGTargetSet() = default;

	explicit RGTargetSet(handle_t data)
		: RGHandle(data)
	{
	}

	bool operator<(const RGTargetSet& rh) const { return m_data < rh.m_data; }

	bool operator>(const RGTargetSet& rh) const { return m_data > rh.m_data; }

	bool operator==(const RGTargetSet& rh) const { return m_data == rh.m_data; }

	bool operator!=(const RGTargetSet& rh) const { return m_data != rh.m_data; }
};

}
