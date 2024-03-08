/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Thread/Acquire.h"
#include "Sound/AudioChannel.h"
#include "Sound/AudioSystem.h"
#include "Sound/Sound.h"
#include "Sound/Filters/GroupFilter.h"
#include "Sound/Filters/LowPassFilter.h"
#include "Sound/Filters/SurroundEnvironment.h"
#include "Sound/Filters/SurroundFilter.h"
#include "Sound/Player/SoundHandle.h"
#include "Sound/Player/SoundListener.h"
#include "Sound/Player/SoundPlayer.h"

namespace traktor::sound
{
	namespace
	{

const float c_nearCutOff = 25000.0f;
const float c_farCutOff = 0.1f;
const float c_recentTimeOffset = 1.0f / 30.0f;

handle_t s_handleDistance = 0;
handle_t s_handleVelocity = 0;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundPlayer", SoundPlayer, ISoundPlayer)

SoundPlayer::SoundPlayer()
{
	s_handleDistance = getParameterHandle(L"Distance");
	s_handleVelocity = getParameterHandle(L"Velocity");
}

bool SoundPlayer::create(AudioSystem* audioSystem, SurroundEnvironment* surroundEnvironment)
{
	m_audioSystem = audioSystem;
	m_surroundEnvironment = surroundEnvironment;

	for (uint32_t i = 0; m_audioSystem->getChannel(i); ++i)
	{
		Channel ch;
		ch.audioChannel = m_audioSystem->getChannel(i);
		ch.priority = ~0U;
		m_channels.push_back(ch);
	}

	m_timer.reset();
	return true;
}

void SoundPlayer::destroy()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	for (auto& channel : m_channels)
	{
		if (channel.handle)
		{
			channel.handle->stop();
			channel.handle = nullptr;
		}
	}

	m_channels.clear();
	m_surroundEnvironment = nullptr;
	m_audioSystem = nullptr;
}

Ref< ISoundHandle > SoundPlayer::play(const Sound* sound, uint32_t priority)
{
	if (!sound)
		return nullptr;

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		const float time = float(m_timer.getElapsedTime());

		// First check if this sound already has been recently played.
		for (const auto& channel : m_channels)
		{
			if (channel.sound == sound && channel.time + c_recentTimeOffset >= time)
				return nullptr;
		}

		// First try to associate sound with non-playing channel.
		for (auto& channel : m_channels)
		{
			if (!channel.audioChannel->isPlaying())
			{
				if (channel.handle)
					channel.handle->detach();

				channel.position = Vector4::zero();
				channel.surroundFilter = nullptr;
				channel.lowPassFilter = nullptr;
				channel.sound = sound;
				channel.audioChannel->play(
					sound->getBuffer(),
					sound->getCategory(),
					sound->getGain(),
					false,
					0
				);
				channel.audioChannel->setFilter(nullptr);
				channel.audioChannel->setVolume(1.0f);
				channel.priority = priority;
				channel.fadeOff = -1.0f;
				channel.time = time;
				channel.autoStopFar = false;
				channel.handle = new SoundHandle(channel.audioChannel, channel.position, channel.fadeOff);

				return channel.handle;
			}
		}

		// Then try to associate sound with lesser priority channel.
		for (auto& channel : m_channels)
		{
			if (priority >= channel.priority)
			{
				if (channel.handle)
					channel.handle->detach();

				channel.position = Vector4::zero();
				channel.surroundFilter = nullptr;
				channel.lowPassFilter = nullptr;
				channel.sound = sound;
				channel.audioChannel->play(
					sound->getBuffer(),
					sound->getCategory(),
					sound->getGain(),
					false,
					0
				);
				channel.audioChannel->setFilter(nullptr);
				channel.audioChannel->setVolume(1.0f);
				channel.priority = priority;
				channel.fadeOff = -1.0f;
				channel.time = time;
				channel.autoStopFar = false;
				channel.handle = new SoundHandle(channel.audioChannel, channel.position, channel.fadeOff);

				return channel.handle;
			}
		}
	}

	return nullptr;
}

Ref< ISoundHandle > SoundPlayer::play(const Sound* sound, const Vector4& position, uint32_t priority, bool autoStopFar)
{
	if (!sound)
		return nullptr;

	if (!m_surroundEnvironment)
		return play(sound, priority);

	const float time = float(m_timer.getElapsedTime());

	float maxDistance = sound->getRange();
	if (maxDistance <= m_surroundEnvironment->getInnerRadius())
		maxDistance = m_surroundEnvironment->getMaxDistance();

	Scalar distance = Scalar(std::numeric_limits< float >::max());
	for (const auto& listenerTransform : m_surroundEnvironment->getListenerTransforms())
	{
		const Vector4 listenerPosition = listenerTransform.translation().xyz1();
		const Scalar listenerDistance = (position - listenerPosition).xyz0().length();
		distance = std::min(distance, listenerDistance);
	}
	if (autoStopFar && distance > maxDistance)
		return nullptr;

	const float k0 = distance / maxDistance;
	const float k1 = (distance - m_surroundEnvironment->getInnerRadius()) / (maxDistance - m_surroundEnvironment->getInnerRadius());

	// Surround filter.
	Ref< SurroundFilter > surroundFilter = new SurroundFilter(m_surroundEnvironment, position.xyz1(), maxDistance);

	// Calculate initial cut-off frequency.
	const float cutOff = lerp(c_nearCutOff, c_farCutOff, clamp(std::sqrt(k0), 0.0f, 1.0f));
	Ref< LowPassFilter > lowPassFilter = new LowPassFilter(cutOff);

	Ref< GroupFilter > groupFilter = new GroupFilter(lowPassFilter, surroundFilter);
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		// First check if this sound already has been recently played.
		//for (const auto& channel : m_channels)
		//{
		//	if (channel.sound == sound && channel.time + c_recentTimeOffset >= time)
		//		return nullptr;
		//}

		// First try to associate sound with non-playing channel.
		for (auto& channel : m_channels)
		{
			const Scalar channelDistance = (channel.position - distance).xyz0().length();
			if (!channel.audioChannel->isPlaying())
			{
				if (channel.handle)
					channel.handle->detach();

				channel.position = position.xyz1();
				channel.surroundFilter = surroundFilter;
				channel.lowPassFilter = lowPassFilter;
				channel.sound = sound;
				channel.audioChannel->play(
					sound->getBuffer(),
					sound->getCategory(),
					sound->getGain(),
					false,
					0
				);
				channel.audioChannel->setFilter(groupFilter);
				channel.audioChannel->setVolume(1.0f);
				channel.priority = priority;
				channel.fadeOff = -1.0f;
				channel.time = time;
				channel.autoStopFar = autoStopFar;
				channel.handle = new SoundHandle(channel.audioChannel, channel.position, channel.fadeOff);

				return channel.handle;
			}
		}

		// Then try to associate sound with lesser priority channel.
		for (auto& channel : m_channels)
		{
			if (priority > channel.priority)
			{
				if (channel.handle)
					channel.handle->detach();

				channel.position = position.xyz1();
				channel.surroundFilter = surroundFilter;
				channel.lowPassFilter = lowPassFilter;
				channel.sound = sound;
				channel.audioChannel->play(
					sound->getBuffer(),
					sound->getCategory(),
					sound->getGain(),
					false,
					0
				);
				channel.audioChannel->setFilter(groupFilter);
				channel.audioChannel->setVolume(1.0f);
				channel.priority = priority;
				channel.fadeOff = -1.0f;
				channel.time = time;
				channel.autoStopFar = autoStopFar;
				channel.handle = new SoundHandle(channel.audioChannel, channel.position, channel.fadeOff);

				return channel.handle;
			}
		}

		// Then try to associate sound with similar priority channel but further away.
		for (auto& channel : m_channels)
		{
			const Scalar channelDistance = (channel.position - distance).xyz0().length();
			if (
				priority == channel.priority &&
				channel.position.w() > 0.5_simd &&
				distance < channelDistance
			)
			{
				if (channel.handle)
					channel.handle->detach();

				channel.position = position.xyz1();
				channel.surroundFilter = surroundFilter;
				channel.lowPassFilter = lowPassFilter;
				channel.sound = sound;
				channel.audioChannel->play(
					sound->getBuffer(),
					sound->getCategory(),
					sound->getGain(),
					false,
					0
				);
				channel.audioChannel->setFilter(groupFilter);
				channel.audioChannel->setVolume(1.0f);
				channel.priority = priority;
				channel.fadeOff = -1.0f;
				channel.time = time;
				channel.autoStopFar = autoStopFar;
				channel.handle = new SoundHandle(channel.audioChannel, channel.position, channel.fadeOff);

				return channel.handle;
			}
		}
	}

	return nullptr;
}

Ref< ISoundListener > SoundPlayer::createListener() const
{
	return new SoundListener();
}

void SoundPlayer::addListener(const ISoundListener* listener)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_listeners.push_back(static_cast< const SoundListener* >(listener));
}

void SoundPlayer::removeListener(const ISoundListener* listener)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_listeners.remove(static_cast< const SoundListener* >(listener));
}

void SoundPlayer::update(float dT)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (m_surroundEnvironment)
	{
		// Update listener transforms.
		SurroundEnvironment::listenerTransformVector_t listenerTransforms;
		for (auto listener : m_listeners)
			listenerTransforms.push_back(listener->getTransform());
		m_surroundEnvironment->setListenerTransforms(listenerTransforms);

		// Update surround and low pass filters on playing 3d sounds.
		for (auto& channel : m_channels)
		{
			// Skip non-playing or non-positional sounds.
			if (!channel.audioChannel->isPlaying() || channel.position.w() < 0.5f)
				continue;

			float maxDistance = channel.sound->getRange();
			if (maxDistance <= m_surroundEnvironment->getInnerRadius())
				maxDistance = m_surroundEnvironment->getMaxDistance();

			// Calculate distance from listener; automatically stop sounds which has moved outside max listener distance.
			Scalar distance = Scalar(std::numeric_limits< float >::max());
			for (const auto& listenerTransform : listenerTransforms)
			{
				const Vector4 listenerPosition = listenerTransform.translation().xyz1();
				const Scalar listenerDistance = (channel.position - listenerPosition).xyz0().length();
				distance = std::min(distance, listenerDistance);
			}
			if (channel.autoStopFar && distance > maxDistance)
			{
				if (channel.handle)
					channel.handle->detach();

				channel.position = Vector4::zero();
				channel.sound = nullptr;
				channel.audioChannel->setFilter(0);
				channel.audioChannel->stop();
				continue;
			}

			// Calculate cut-off frequency.
			const float k0 = clamp< float >(distance / maxDistance, 0.0f, 1.0f);

			// Set filter parameters.
			if (channel.surroundFilter)
				channel.surroundFilter->setSpeakerPosition(channel.position);
			if (channel.lowPassFilter)
			{
				const float cutOff = lerp(c_nearCutOff, c_farCutOff, std::sqrt(k0));
				channel.lowPassFilter->setCutOff(cutOff);
			}

			// Set automatic sound parameters.
			channel.audioChannel->setParameter(s_handleDistance, k0);
			channel.audioChannel->setParameter(s_handleVelocity, 0.0f);

			// Disable repeat if no-one else then me have a reference to the handle.
			if (!channel.handle || channel.handle->getReferenceCount() <= 1)
				channel.audioChannel->disableRepeat();
		}
	}

	// Update fade-off channels.
	for (auto& channel : m_channels)
	{
		if (!channel.audioChannel->isPlaying() || channel.fadeOff <= 0.0f)
			continue;

		channel.fadeOff -= std::min(dT, 1.0f / 60.0f);
		if (channel.fadeOff > 0.0f)
			channel.audioChannel->setVolume(channel.fadeOff);
		else
			channel.audioChannel->stop();
	}
}

}
