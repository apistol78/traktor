#include "Runtime/IEnvironment.h"
#include "Runtime/UpdateInfo.h"
#include "Runtime/Engine/AudioLayer.h"
#include "Core/Math/Float.h"
#include "Core/Timer/Profiler.h"
#include "Sound/Sound.h"
#include "Sound/Player/ISoundHandle.h"
#include "Sound/Player/ISoundPlayer.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.AudioLayer", AudioLayer, Layer)

AudioLayer::AudioLayer(
	Stage* stage,
	const std::wstring& name,
	bool permitTransition,
	IEnvironment* environment,
	const resource::Proxy< sound::Sound >& sound,
	bool autoPlay,
	bool repeat
)
:	Layer(stage, name, permitTransition)
,	m_environment(environment)
,	m_sound(sound)
,	m_autoPlay(autoPlay)
,	m_repeat(repeat)
{
}

void AudioLayer::destroy()
{
	m_environment = nullptr;
	m_sound.clear();
	m_tweens.clear();

	if (m_handle)
	{
		m_handle->fadeOff();
		m_handle = nullptr;
	}

	Layer::destroy();
}

void AudioLayer::play()
{
	if (m_handle && m_handle->isPlaying())
		return;

	sound::ISoundPlayer* soundPlayer = m_environment->getAudio()->getSoundPlayer();
	if (soundPlayer)
		m_handle = soundPlayer->play(m_sound, 0);
	else
		m_handle = nullptr;
}

void AudioLayer::stop()
{
	if (m_handle)
	{
		m_handle->stop();
		m_handle = nullptr;
	}
}

void AudioLayer::fadeOff()
{
	if (m_handle)
	{
		m_handle->fadeOff();
		m_handle = nullptr;
	}
}

void AudioLayer::setParameter(const std::wstring& parameterName, float value)
{
	if (m_handle)
		m_handle->setParameter(sound::getParameterHandle(parameterName), value);
}

void AudioLayer::tweenParameter(const std::wstring& parameterName, float fromValue, float toValue, float duration)
{
	Tween& tween = m_tweens.push_back();
	tween.parameter = sound::getParameterHandle(parameterName);
	tween.fromValue = fromValue;
	tween.toValue = toValue;
	tween.duration = duration;
	tween.time = 0.0f;
}

void AudioLayer::transition(Layer* fromLayer)
{
	bool permit = fromLayer->isTransitionPermitted() && isTransitionPermitted();
	if (!permit)
		return;

	AudioLayer* fromAudioLayer = checked_type_cast< AudioLayer*, false >(fromLayer);
	if (m_sound == fromAudioLayer->m_sound)
	{
		m_handle = fromAudioLayer->m_handle;
		m_tweens = fromAudioLayer->m_tweens;
		fromAudioLayer->m_handle = nullptr;
		fromAudioLayer->m_tweens.clear();
	}
}

void AudioLayer::prepare(const UpdateInfo& info)
{
}

void AudioLayer::update(const UpdateInfo& info)
{
	T_PROFILER_SCOPE(L"AudioLayer update");

	if (
		m_autoPlay &&
		(!m_handle || !m_handle->isPlaying())
	)
	{
		sound::ISoundPlayer* soundPlayer = m_environment->getAudio()->getSoundPlayer();
		if (soundPlayer)
			m_handle = soundPlayer->play(m_sound, 0);

		if (!m_repeat)
			m_autoPlay = false;
	}

	if (m_handle)
	{
		for (AlignedVector< Tween >::iterator i = m_tweens.begin(); i != m_tweens.end(); )
		{
			if (i->time <= i->duration)
			{
				float k = i->time / i->duration;
				m_handle->setParameter(i->parameter, lerp(i->fromValue, i->toValue, k));
				i->time += info.getSimulationDeltaTime();
				++i;
			}
			else
			{
				m_handle->setParameter(i->parameter, i->toValue);
				i = m_tweens.erase(i);
			}
		}
	}
}

void AudioLayer::setup(const UpdateInfo& info, render::RenderGraph& renderGraph)
{
}

void AudioLayer::preReconfigured()
{
}

void AudioLayer::postReconfigured()
{
}

void AudioLayer::suspend()
{
}

void AudioLayer::resume()
{
}

	}
}
