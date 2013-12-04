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
:	m_format(VfUnknown)
,	m_time(0.0f)
,	m_rate(0.0f)
,	m_frame(0)
,	m_current(0)
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

	for (uint32_t i = 0; i < sizeof_array(m_textures); ++i)
	{
		m_textures[i] = renderSystem->createSimpleTexture(desc);
		if (!m_textures[i])
			return false;
	}

	m_decoder = decoder;
	m_format = info.format;
	m_time = 0.0f;
	m_rate = info.rate;
	m_frame = ~0U;
	m_current = 0;
	m_playing = true;

	return true;
}

void Video::destroy()
{
	for (uint32_t i = 0; i < sizeof_array(m_textures); ++i)
		safeDestroy(m_textures[i]);
	m_decoder = 0;
	m_playing = false;
}

bool Video::update(float deltaTime)
{
	uint32_t frame = uint32_t(m_rate * m_time);
	if (frame != m_frame)
	{
		uint32_t next = (m_current + 1) % sizeof_array(m_textures);
		render::ISimpleTexture* texture = m_textures[next];

		render::ITexture::Lock lock;
		if (texture->lock(0, lock))
		{
			m_playing = m_decoder->decode(frame, lock.bits, lock.pitch);
			texture->unlock(0);

			m_frame = frame;
			m_current = next;
		}
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
	m_time = 0.0f;
	m_frame = ~0U;
	m_current = 0;
	m_playing = true;
}

render::ISimpleTexture* Video::getTexture()
{
	return m_textures[m_current];
}

VideoFormat Video::getFormat() const
{
	return m_format;
}

	}
}
