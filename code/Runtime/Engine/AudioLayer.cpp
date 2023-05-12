/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/IEnvironment.h"
#include "Runtime/UpdateInfo.h"
#include "Runtime/Engine/AudioLayer.h"
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Timer/Profiler.h"
#include "Sound/Sound.h"
#include "Sound/Player/ISoundHandle.h"
#include "Sound/Player/ISoundPlayer.h"

namespace traktor::runtime
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
	m_sound.consume();
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
	const sound::handle_t handle = sound::getParameterHandle(parameterName);
	Tween& tween = m_tweens[handle];
	tween.fromValue = value;
	tween.toValue = value;
	tween.lastValue = value;
	tween.duration = 0.0f;
	tween.time = 0.0f;
}

void AudioLayer::tweenParameter(const std::wstring& parameterName, float toValue, float duration)
{
	const sound::handle_t handle = sound::getParameterHandle(parameterName);
	Tween& tween = m_tweens[handle];
	tween.fromValue = tween.lastValue;
	tween.toValue = toValue;
	tween.duration = duration;
	tween.time = 0.0f;
}

void AudioLayer::tweenParameter(const std::wstring& parameterName, float fromValue, float toValue, float duration)
{
	const sound::handle_t handle = sound::getParameterHandle(parameterName);
	Tween& tween = m_tweens[handle];
	tween.fromValue = fromValue;
	tween.toValue = toValue;
	tween.lastValue = fromValue;
	tween.duration = duration;
	tween.time = 0.0f;
}

void AudioLayer::transition(Layer* fromLayer)
{
	const bool permit = fromLayer->isTransitionPermitted() && isTransitionPermitted();
	if (!permit)
		return;

	AudioLayer* fromAudioLayer = checked_type_cast< AudioLayer*, false >(fromLayer);
	if (m_sound == fromAudioLayer->m_sound)
	{
		m_handle = fromAudioLayer->m_handle;
		m_tweens = fromAudioLayer->m_tweens;
		fromAudioLayer->m_handle = nullptr;
		fromAudioLayer->m_tweens.clear();
		m_sound.consume();
	}
}

void AudioLayer::preUpdate(const UpdateInfo& info)
{
}

void AudioLayer::update(const UpdateInfo& info)
{
	T_PROFILER_SCOPE(L"AudioLayer update");

	if (m_sound.changed())
	{
		if (m_handle != nullptr)
		{
			m_handle->stop();
			m_handle = nullptr;
		}
		m_sound.consume();
	}

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
		for (auto& it : m_tweens)
		{
			auto& tween = it.second;
			if (tween.time <= tween.duration)
			{
				if (tween.duration >= FUZZY_EPSILON)
				{
					const float k = tween.time / tween.duration;
					tween.lastValue = lerp(tween.fromValue, tween.toValue, k);
				}
				else
					tween.lastValue = tween.toValue;

				m_handle->setParameter(it.first, tween.lastValue);

				tween.time += info.getFrameDeltaTime();
			}
		}
	}
}

void AudioLayer::preSetup(const UpdateInfo& info)
{
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

void AudioLayer::hotReload()
{
}

}
