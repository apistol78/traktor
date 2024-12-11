/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/ITexture.h"

namespace traktor
{

class IStream;

}

namespace traktor::render
{

class IRenderSystem;
class ITexture;

class VirtualTexture : public ITexture
{
	T_RTTI_CLASS;

public:
	explicit VirtualTexture(IRenderSystem* renderSystem, IStream* stream, ITexture* lowQualityMipTexture);

	virtual void destroy() override final;

	virtual Size getSize() const override final;

	virtual int32_t getBindlessIndex() const override final;

	virtual bool lock(int32_t side, int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t side, int32_t level) override final;

	virtual ITexture* resolve() override final;

private:
	Ref< IRenderSystem > m_renderSystem;

	Ref< IStream > m_stream;
	Ref< ITexture > m_lowQualityMipTexture;
	Ref< ITexture > m_highQualityMipTexture;

	int32_t m_imageWidth;
	int32_t m_imageHeight;
	int32_t m_mipCount;
	int32_t m_texelFormat;
	bool m_sRGB;
	bool m_compressed;

	int64_t m_textureDataOffset;
	bool m_highQualityLoadedOrPending;

	bool loadHighQuality();
};

}
