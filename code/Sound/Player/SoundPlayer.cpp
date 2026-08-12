/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
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

/*! Position of a source within the fade band; 0 at the inner radius, 1 at max distance.
 *
 * Same normalization as the distance attenuation of the surround filter, so a
 * source within the inner radius is heard at both full volume and full
 * bandwidth, and reach silence and full muffling at the same distance.
 */
float fadeBandRatio(float distance, float innerRadius, float maxDistance)
{
	// Guard against an inner radius which reach beyond the max distance.
	float fadeRange = maxDistance - innerRadius;
	if (fadeRange < FUZZY_EPSILON)
		fadeRange = FUZZY_EPSILON;

	return clamp((distance - innerRadius) / fadeRange, 0.0f, 1.0f);
}

/*! Distance from the closest listener to a position.
 *
 * \param surroundEnvironment Environment holding the listener transforms.
 * \param position Position in world space.
 * \return Scaled distance to the closest listener; infinite when there are no listeners.
 */
Scalar closestListenerDistance(const SurroundEnvironment* surroundEnvironment, const Vector4& position)
{
	Scalar distance = Scalar(std::numeric_limits< float >::max());
	for (const auto& listenerTransform : surroundEnvironment->getListenerTransforms())
	{
		const Vector4 listenerPosition = listenerTransform.translation().xyz1();
		distance = std::min(distance, surroundEnvironment->getScaledDistance(position - listenerPosition));
	}
	return distance;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundPlayer", SoundPlayer, Object)

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

Ref< SoundHandle > SoundPlayer::play(const Sound* sound, uint32_t priority)
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

Ref< SoundHandle > SoundPlayer::play(const Sound* sound, const Vector4& position, uint32_t priority, bool autoStopFar)
{
	if (!sound)
		return nullptr;

	if (!m_surroundEnvironment)
		return play(sound, priority);

	const float time = float(m_timer.getElapsedTime());

	float maxDistance = sound->getRange();
	if (maxDistance <= m_surroundEnvironment->getInnerRadius())
		maxDistance = m_surroundEnvironment->getMaxDistance();

	const Scalar distance = closestListenerDistance(m_surroundEnvironment, position.xyz1());
	if (autoStopFar && distance > maxDistance)
		return nullptr;

	// Surround filter.
	Ref< SurroundFilter > surroundFilter = new SurroundFilter(m_surroundEnvironment, position.xyz1(), maxDistance);

	// Calculate initial cut-off frequency.
	const float k = fadeBandRatio(distance, m_surroundEnvironment->getInnerRadius(), maxDistance);
	const float cutOff = lerp(c_nearCutOff, c_farCutOff, std::sqrt(k));
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
			// Steal from a channel whose own source is further away than this one.
			const Scalar channelDistance = closestListenerDistance(m_surroundEnvironment, channel.position);
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

void SoundPlayer::addListener(const SoundListener* listener)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	// Listener transforms reach the surround environment through a fixed size vector, so
	// refuse here instead of overrunning it later; push_back past the capacity is only an
	// assert, which means a release build writes outside the vector. Hitting this usually
	// means listeners are being leaked rather than that this many are really wanted.
	if (m_listeners.size() >= SurroundEnvironment::listenerTransformVector_t::Capacity)
	{
		log::error << L"Unable to add sound listener; at most " << int32_t(SurroundEnvironment::listenerTransformVector_t::Capacity) << L" listeners are supported." << Endl;
		return;
	}

	m_listeners.push_back(listener);
}

void SoundPlayer::removeListener(const SoundListener* listener)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_listeners.remove(listener);
}

void SoundPlayer::update(float dT)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (m_surroundEnvironment)
	{
		// Update listener transforms.
		SurroundEnvironment::listenerTransformVector_t listenerTransforms;
		for (auto listener : m_listeners)
		{
			if (listenerTransforms.full())
				break;
			listenerTransforms.push_back(listener->getTransform());
		}
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
			const Scalar distance = closestListenerDistance(m_surroundEnvironment, channel.position);
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
			const float k = fadeBandRatio(distance, m_surroundEnvironment->getInnerRadius(), maxDistance);

			// Set filter parameters.
			if (channel.surroundFilter)
				channel.surroundFilter->setSpeakerPosition(channel.position);
			if (channel.lowPassFilter)
			{
				const float cutOff = lerp(c_nearCutOff, c_farCutOff, std::sqrt(k));
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
