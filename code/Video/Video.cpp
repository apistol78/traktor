/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include "Core/Functor/Functor.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Render/IRenderSystem.h"
#include "Render/ISimpleTexture.h"
#include "Video/IVideoDecoder.h"
#include "Video/Video.h"

namespace traktor
{
	namespace video
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.video.Video", Video, Object)

Video::Video()
:	m_time(0.0f)
,	m_rate(0.0f)
,	m_playing(false)
{
}

bool Video::create(render::IRenderSystem* renderSystem, IVideoDecoder* decoder)
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
		m_textures[i] = renderSystem->createSimpleTexture(desc);
		if (!m_textures[i])
			return false;
	}

	m_decoder = decoder;
	m_time = 0.0f;
	m_rate = info.rate;

	m_frameBuffer.reset((uint8_t*)Alloc::acquireAlign(info.width * info.height * 4, 16, T_FILE_LINE));
	m_frameBufferPitch = info.width * 4;
	m_frameBufferSize = info.width * info.height * 4;

	m_playing = true;
	m_lastDecodedFrame = ~0U;
	m_lastUploadedFrame = ~0U;
	m_current = 0;

	update(0.0f);
	return true;
}

Video::~Video()
{
	destroy();
}

void Video::destroy()
{
	if (m_job)
	{
		m_job->wait();
		m_job = 0;
	}

	for (uint32_t i = 0; i < sizeof_array(m_textures); ++i)
		safeDestroy(m_textures[i]);

	m_decoder = 0;
	m_playing = false;
	m_frameBuffer.release();
}

bool Video::update(float deltaTime)
{
	if (!m_playing)
		return false;

	uint32_t frame = uint32_t(m_rate * m_time);
	if (frame != m_lastDecodedFrame)
	{
		// Wait for previous decode job to finish.
		if (m_job)
		{
			m_job->wait();
			m_job = 0;
		}

		// Enqueue new decoding job.
		m_job = JobManager::getInstance().add(makeFunctor(
			this,
			&Video::decodeFrame,
			frame
		));

		m_lastDecodedFrame = frame;
	}

	m_time += deltaTime;
	return m_playing;
}

bool Video::playing() const
{
	return m_playing;
}

void Video::rewind()
{
	m_decoder->rewind();

	m_time = 0.0f;
	m_playing = true;
	m_lastDecodedFrame = ~0U;
	m_lastUploadedFrame = ~0U;

	update(0.0f);
}

render::ISimpleTexture* Video::getTexture()
{
	if (m_lastUploadedFrame != m_lastDecodedFrame)
	{
		m_current = (m_current + 1) % sizeof_array(m_textures);
		render::ISimpleTexture* texture = m_textures[m_current];
		render::ITexture::Lock lock;
		if (texture->lock(0, lock))
		{
			const uint8_t* s = m_frameBuffer.c_ptr();
			uint8_t* d = static_cast< uint8_t* >(lock.bits);

			uint32_t rows = m_frameBufferSize / m_frameBufferPitch;
			for (uint32_t y = 0; y < rows; ++y)
			{
				std::memcpy(d, s, m_frameBufferPitch);
				s += m_frameBufferPitch;
				d += lock.pitch;
			}

			texture->unlock(0);
		}
		m_lastUploadedFrame = m_lastDecodedFrame;
		return texture;
	}
	else
		return m_textures[m_current];
}

void Video::decodeFrame(uint32_t frame)
{
	m_playing = m_decoder->decode(frame, m_frameBuffer.ptr(), m_frameBufferPitch);
}

	}
}
