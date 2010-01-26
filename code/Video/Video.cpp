#include "Core/Misc/SafeDestroy.h"
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
,	m_frame(0)
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
	desc.immutable = false;

	m_texture = renderSystem->createSimpleTexture(desc);
	if (!m_texture)
		return false;

	m_decoder = decoder;
	m_time = 0.0f;
	m_rate = info.rate;
	m_frame = ~0UL;
	m_playing = true;

	return true;
}

void Video::destroy()
{
	safeDestroy(m_texture);
	m_decoder = 0;
}

bool Video::update(float deltaTime)
{
	uint32_t frame = uint32_t(m_rate * m_time);
	if (frame != m_frame)
	{
		render::ITexture::Lock lock;
		if (m_texture->lock(0, lock))
		{
			m_playing = m_decoder->decode(frame, lock.bits, lock.pitch);
			m_texture->unlock(0);

			m_frame = frame;
		}
	}
	m_time += deltaTime;
	return m_playing;
}

bool Video::playing() const
{
	return m_playing;
}

render::ISimpleTexture* Video::getTexture()
{
	return m_texture;
}

	}
}
