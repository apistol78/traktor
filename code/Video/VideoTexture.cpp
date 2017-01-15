#include <cstring>
#include "Core/Functor/Functor.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadPool.h"
#include "Render/IRenderSystem.h"
#include "Render/ISimpleTexture.h"
#include "Video/IVideoDecoder.h"
#include "Video/VideoTexture.h"

namespace traktor
{
	namespace video
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.video.VideoTexture", VideoTexture, render::ISimpleTexture)

VideoTexture::VideoTexture()
:	m_rate(0.0f)
,	m_frameBufferPitch(0)
,	m_frameBufferSize(0)
,	m_lastDecodedFrame(0)
,	m_lastUploadedFrame(0)
,	m_current(0)
,	m_thread(0)
{
}

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
		m_textures[i] = renderSystem->createSimpleTexture(desc);
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

	m_timer.start();

	if (!ThreadPool::getInstance().spawn(makeFunctor(this, &VideoTexture::decodeThread), m_thread))
		return false;

	return true;
}

void VideoTexture::destroy()
{
	if (m_thread)
	{
		ThreadPool::getInstance().stop(m_thread);
		m_thread = 0;
	}

	for (uint32_t i = 0; i < sizeof_array(m_textures); ++i)
		safeDestroy(m_textures[i]);
}

render::ITexture* VideoTexture::resolve()
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

int VideoTexture::getWidth() const
{
	return m_textures[0]->getWidth();
}

int VideoTexture::getHeight() const
{
	return m_textures[0]->getHeight();
}

bool VideoTexture::lock(int level, render::ITexture::Lock& lock)
{
	return false;
}

void VideoTexture::unlock(int level)
{
}

void* VideoTexture::getInternalHandle()
{
	return 0;
}

void VideoTexture::decodeThread()
{
	while (!m_thread->stopped())
	{
		uint32_t frame = uint32_t(m_rate * m_timer.getElapsedTime());
		if (frame != m_lastDecodedFrame)
		{
			bool playing = m_decoder->decode(frame, m_frameBuffer.ptr(), m_frameBufferPitch);
			if (!playing)
			{
				m_decoder->rewind();
				m_timer.start();
				continue;
			}
			m_lastDecodedFrame = frame;
		}

		m_thread->sleep(250 / m_rate);
	}
}

	}
}
