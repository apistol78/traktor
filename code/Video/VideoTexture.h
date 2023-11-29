/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Misc/AutoPtr.h"
#include "Core/Timer/Timer.h"
#include "Render/ITexture.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_VIDEO_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Thread;

}

namespace traktor::render
{

class IRenderSystem;

}

namespace traktor::video
{

class IVideoDecoder;

class T_DLLCLASS VideoTexture : public render::ITexture
{
	T_RTTI_CLASS;

public:
	virtual ~VideoTexture();

	bool create(render::IRenderSystem* renderSystem, IVideoDecoder* decoder);

	virtual void destroy() override final;

	virtual Size getSize() const override final;

	virtual bool lock(int32_t side, int32_t level, render::ITexture::Lock& lock) override final;

	virtual void unlock(int32_t side, int32_t level) override final;

	virtual render::ITexture* resolve() override final;

private:
	Ref< IVideoDecoder > m_decoder;
	Ref< render::ITexture > m_textures[4];
	Timer m_timer;
	float m_rate = 0.0f;
	AutoPtr< uint8_t, AllocFreeAlign > m_frameBuffer;
	uint32_t m_frameBufferPitch = 0;
	uint32_t m_frameBufferSize = 0;
	uint32_t m_lastDecodedFrame = 0;
	uint32_t m_lastUploadedFrame = 0;
	uint32_t m_current = 0;
	Thread* m_thread = nullptr;

	void decodeThread();
};

}
