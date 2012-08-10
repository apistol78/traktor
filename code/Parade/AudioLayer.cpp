#include "Parade/AudioLayer.h"
#include "Sound/Sound.h"
#include "Sound/SoundChannel.h"
#include "Sound/SoundSystem.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.AudioLayer", AudioLayer, Layer)

AudioLayer::AudioLayer(
	const std::wstring& name,
	amalgam::IEnvironment* environment,
	const resource::Proxy< script::IScriptContext >& scriptContext,
	const resource::Proxy< sound::Sound >& sound
)
:	Layer(name, scriptContext)
,	m_environment(environment)
,	m_sound(sound)
{
}

void AudioLayer::update(Stage* stage, amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info)
{
	// Issue script update method.
	invokeScriptUpdate(stage, control, info);

	// Play sound if not currently attached.
	if (!m_soundChannel)
		m_soundChannel = m_environment->getAudio()->getSoundSystem()->play(m_sound, 0, false, ~0UL);
}

void AudioLayer::build(Stage* stage, const amalgam::IUpdateInfo& info, uint32_t frame)
{
}

void AudioLayer::render(Stage* stage, render::EyeType eye, uint32_t frame)
{
}

void AudioLayer::leave(Stage* stage)
{
	if (m_soundChannel)
	{
		m_soundChannel->stop();
		m_soundChannel = 0;
	}
}

void AudioLayer::reconfigured(Stage* stage)
{
}

	}
}
