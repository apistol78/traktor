/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadPool.h"
#include "Render/IRenderSystem.h"
#include "Video/IVideoDecoder.h"
#include "Video/VideoTexture.h"

namespace traktor::video
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.video.VideoTexture", VideoTexture, render::ITexture)

VideoTexture::~VideoTexture()
{
	destroy();
}

bool VideoTexture::create(render::IRenderSystem* renderSystem, IVideoDecoder* decoder)
{
	VideoDecoderInfo info;
	decoder->getInformation(info);

	render::SimpleTextureCreateDesc desc;
	desc.width = info.width;
	desc.height = info.height;
	desc.mipCount = 1;
	desc.format = render::TfR8G8B8A8;
	desc.sRGB = false;
	desc.immutable = false;

	for (uint32_t i = 0; i < sizeof_array(m_textures); ++i)
	{
		m_textures[i] = renderSystem->createSimpleTexture(desc, T_FILE_LINE_W);
		if (!m_textures[i])
			return false;
	}

	m_decoder = decoder;
	m_rate = info.rate;

	m_frameBuffer.reset((uint8_t*)Alloc::acquireAlign(info.width * info.height * 4, 16, T_FILE_LINE));
	m_frameBufferPitch = info.width * 4;
	m_frameBufferSize = info.width * info.height * 4;

	m_lastDecodedFrame = ~0U;
	m_lastUploadedFrame = ~0U;
	m_current = 0;

	m_timer.reset();

	if (!ThreadPool::getInstance().spawn([=, this](){ decodeThread(); }, m_thread))
		return false;

	return true;
}

void VideoTexture::destroy()
{
	if (m_thread)
	{
		ThreadPool::getInstance().stop(m_thread);
		m_thread = nullptr;
	}

	for (uint32_t i = 0; i < sizeof_array(m_textures); ++i)
		safeDestroy(m_textures[i]);
}

render::ITexture::Size VideoTexture::getSize() const
{
	return m_textures[0]->getSize();
}

bool VideoTexture::lock(int32_t side, int32_t level, render::ITexture::Lock& lock)
{
	return false;
}

void VideoTexture::unlock(int32_t side, int32_t level)
{
}

render::ITexture* VideoTexture::resolve()
{
	if (m_lastUploadedFrame != m_lastDecodedFrame)
	{
		m_current = (m_current + 1) % sizeof_array(m_textures);
		render::ITexture* texture = m_textures[m_current];
		render::ITexture::Lock lock;
		if (texture->lock(0, 0, lock))
		{
			const uint8_t* s = m_frameBuffer.c_ptr();
			uint8_t* d = static_cast< uint8_t* >(lock.bits);

			const uint32_t rows = m_frameBufferSize / m_frameBufferPitch;
			for (uint32_t y = 0; y < rows; ++y)
			{
				std::memcpy(d, s, m_frameBufferPitch);
				s += m_frameBufferPitch;
				d += lock.pitch;
			}

			texture->unlock(0, 0);
		}
		m_lastUploadedFrame = m_lastDecodedFrame;
		return texture;
	}
	else
		return m_textures[m_current];
}

void VideoTexture::decodeThread()
{
	while (!m_thread->stopped())
	{
		const uint32_t frame = uint32_t(m_rate * m_timer.getElapsedTime());
		if (frame != m_lastDecodedFrame)
		{
			const bool playing = m_decoder->decode(frame, m_frameBuffer.ptr(), m_frameBufferPitch);
			if (!playing)
			{
				m_decoder->rewind();
				m_timer.reset();
				continue;
			}
			m_lastDecodedFrame = frame;
		}
		m_thread->sleep((int32_t)(250.0f / m_rate));
	}
}

}
