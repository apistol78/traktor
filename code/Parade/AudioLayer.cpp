#include "Parade/AudioLayer.h"
#include "Sound/Sound.h"
#include "Sound/Player/ISoundHandle.h"
#include "Sound/Player/ISoundPlayer.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.AudioLayer", AudioLayer, Layer)

AudioLayer::AudioLayer(
	Stage* stage,
	const std::wstring& name,
	amalgam::IEnvironment* environment,
	const resource::Proxy< sound::Sound >& sound
)
:	Layer(stage, name)
,	m_environment(environment)
,	m_sound(sound)
{
}

void AudioLayer::prepare()
{
}

void AudioLayer::update(amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info)
{
	// Play sound if not currently attached.
	if (!m_handle || !m_handle->isPlaying())
	{
		sound::ISoundPlayer* soundPlayer = m_environment->getAudio()->getSoundPlayer();
		if (soundPlayer)
			m_handle = soundPlayer->play(m_sound, 0);
	}
}

void AudioLayer::build(const amalgam::IUpdateInfo& info, uint32_t frame)
{
}

void AudioLayer::render(render::EyeType eye, uint32_t frame)
{
}

void AudioLayer::leave()
{
	if (m_handle)
	{
		m_handle->stop();
		m_handle = 0;
	}
}

void AudioLayer::reconfigured()
{
}

	}
}
