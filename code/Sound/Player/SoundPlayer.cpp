#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Thread/Acquire.h"
#include "Sound/Sound.h"
#include "Sound/SoundChannel.h"
#include "Sound/SoundSystem.h"
#include "Sound/Filters/EchoFilter.h"
#include "Sound/Filters/GroupFilter.h"
#include "Sound/Filters/LowPassFilter.h"
#include "Sound/Filters/SurroundEnvironment.h"
#include "Sound/Filters/SurroundFilter.h"
#include "Sound/Player/SoundHandle.h"
#include "Sound/Player/SoundPlayer.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const float c_nearCutOff = 22050.0f;
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

bool SoundPlayer::create(SoundSystem* soundSystem, SurroundEnvironment* surroundEnvironment)
{
	m_soundSystem = soundSystem;
	m_surroundEnvironment = surroundEnvironment;

	for (uint32_t i = 0; m_soundSystem->getChannel(i); ++i)
	{
		Channel ch;
		ch.soundChannel = m_soundSystem->getChannel(i);
		ch.priority = ~0UL;
		m_channels.push_back(ch);
	}

	m_timer.start();
	return true;
}

void SoundPlayer::destroy()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	for (AlignedVector< Channel >::iterator i = m_channels.begin(); i != m_channels.end(); ++i)
	{
		if (i->handle)
		{
			i->handle->stop();
			i->handle = 0;
		}
	}

	m_channels.clear();
	m_surroundEnvironment = 0;
	m_soundSystem = 0;
}

Ref< ISoundHandle > SoundPlayer::play(const Sound* sound, uint32_t priority)
{
	if (!sound)
		return 0;

	float time = float(m_timer.getElapsedTime());

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		// First try to associate sound with non-playing channel;
		// also check if sound has recently been played.
		for (AlignedVector< Channel >::iterator i = m_channels.begin(); i != m_channels.end(); ++i)
		{
			if (!i->soundChannel->isPlaying())
			{
				if (i->handle)
					i->handle->detach();

				i->position = Vector4::zero();
				i->surroundFilter = 0;
				i->lowPassFilter = 0;
				i->echoFilter = 0;
				i->sound = sound;
				i->soundChannel->play(
					sound->getBuffer(),
					sound->getCategory(),
					sound->getVolume(),
					sound->getPresence(),
					sound->getPresenceRate()
				);
				i->soundChannel->setFilter(0);
				i->soundChannel->setVolume(1.0f);
				i->priority = priority;
				i->time = time;
				i->fadeOff = -1.0f;
				i->handle = new SoundHandle(i->soundChannel, i->position, i->fadeOff);

				return i->handle;
			}
			else if (i->time + c_recentTimeOffset >= time)
			{
				if (i->sound == sound)
				{
					i->time = time;
					i->fadeOff = -1.0f;
					return 0;
				}
			}
		}

		// Then try to associate sound with lesser priority channel.
		for (AlignedVector< Channel >::iterator i = m_channels.begin(); i != m_channels.end(); ++i)
		{
			if (priority >= i->priority)
			{
				if (i->handle)
					i->handle->detach();

				i->position = Vector4::zero();
				i->surroundFilter = 0;
				i->lowPassFilter = 0;
				i->echoFilter = 0;
				i->sound = sound;
				i->soundChannel->play(
					sound->getBuffer(),
					sound->getCategory(),
					sound->getVolume(),
					sound->getPresence(),
					sound->getPresenceRate()
				);
				i->soundChannel->setFilter(0);
				i->soundChannel->setVolume(1.0f);
				i->priority = priority;
				i->time = time;
				i->fadeOff = -1.0f;
				i->handle = new SoundHandle(i->soundChannel, i->position, i->fadeOff);

				return i->handle;
			}
		}
	}

	return 0;
}

Ref< ISoundHandle > SoundPlayer::play3d(const Sound* sound, const Vector4& position, uint32_t priority)
{
	if (!sound)
		return 0;

	if (!m_surroundEnvironment)
		return play(sound, priority);

	float maxDistance = sound->getRange();
	if (maxDistance <= m_surroundEnvironment->getInnerRadius())
		maxDistance = m_surroundEnvironment->getMaxDistance();

	Vector4 listenerPosition = m_surroundEnvironment->getListenerTransform().translation().xyz1();

	// Calculate distance from listener.
	Scalar distance = (position - listenerPosition).xyz0().length();
	if (distance > maxDistance)
		return 0;

	float k0 = distance / maxDistance;
	float k1 = distance / m_surroundEnvironment->getInnerRadius();
	float k2 = (distance - m_surroundEnvironment->getInnerRadius()) / (maxDistance - m_surroundEnvironment->getInnerRadius());

	// Surround filter.
	Ref< SurroundFilter > surroundFilter = new SurroundFilter(m_surroundEnvironment, position.xyz1());

	// Calculate initial cut-off frequency.
	float cutOff = lerp(c_nearCutOff, c_farCutOff, clamp(std::sqrt(k0), 0.0f, 1.0f));
	Ref< LowPassFilter > lowPassFilter = new LowPassFilter(cutOff);

	// Calculate presence; further sounds have less presence.
	// As long as the sound originate inside inner radius then
	// original presence is kept.
	float presence = lerp(sound->getPresence(), 0.0f, clamp(k2, 0.0f, 1.0f));

	// Calculate echo.
	Ref< EchoFilter > echoFilter;
	float delay = 0.4f * clamp((k0 - 0.2f) / 0.2f, 0.0f, 1.0f);
	if (delay > 0.15f)
	{
		echoFilter = new EchoFilter(
			delay,
			0.2f,
			0.7f,
			0.5f
		);
	}

	// Create group filter.
	Ref< GroupFilter > groupFilter;
	if (echoFilter)
		groupFilter = new GroupFilter(lowPassFilter, echoFilter, surroundFilter);
	else
		groupFilter = new GroupFilter(lowPassFilter, surroundFilter);

	float time = float(m_timer.getElapsedTime());

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		// First try to associate sound with non-playing channel (or far away);
		// also check if sound has recently been played.
		for (AlignedVector< Channel >::iterator i = m_channels.begin(); i != m_channels.end(); ++i)
		{
			Scalar channelDistance = (i->position - listenerPosition).xyz0().length();
			if (
				!i->soundChannel->isPlaying() ||
				(i->position.w() > 0.5f && channelDistance > m_surroundEnvironment->getMaxDistance())
			)
			{
				if (i->handle)
					i->handle->detach();

				i->position = position.xyz1();
				i->surroundFilter = surroundFilter;
				i->lowPassFilter = lowPassFilter;
				i->echoFilter = echoFilter;
				i->sound = sound;
				i->soundChannel->play(
					sound->getBuffer(),
					sound->getCategory(),
					sound->getVolume(),
					presence,
					sound->getPresenceRate()
				);
				i->soundChannel->setFilter(groupFilter);
				i->soundChannel->setVolume(1.0f);
				i->priority = priority;
				i->time = time;
				i->fadeOff = -1.0f;
				i->handle = new SoundHandle(i->soundChannel, i->position, i->fadeOff);

				return i->handle; 
			}
			else if (i->time + c_recentTimeOffset >= time)
			{
				if (i->sound == sound)
				{
					i->time = time;
					i->fadeOff = -1.0f;
					return 0;
				}
			}
		}

		// Then try to associate sound with lesser priority channel.
		for (AlignedVector< Channel >::iterator i = m_channels.begin(); i != m_channels.end(); ++i)
		{
			if (priority > i->priority)
			{
				if (i->handle)
					i->handle->detach();

				i->position = position.xyz1();
				i->surroundFilter = surroundFilter;
				i->lowPassFilter = lowPassFilter;
				i->echoFilter = echoFilter;
				i->sound = sound;
				i->soundChannel->play(
					sound->getBuffer(),
					sound->getCategory(),
					sound->getVolume(),
					presence,
					sound->getPresenceRate()
				);
				i->soundChannel->setFilter(groupFilter);
				i->soundChannel->setVolume(1.0f);
				i->priority = priority;
				i->time = time;
				i->fadeOff = -1.0f;
				i->handle = new SoundHandle(i->soundChannel, i->position, i->fadeOff);

				return i->handle;
			}
		}

		// Then try to associate sound with similar priority channel but further away.
		for (AlignedVector< Channel >::iterator i = m_channels.begin(); i != m_channels.end(); ++i)
		{
			Scalar channelDistance = (i->position - listenerPosition).xyz0().length();
			if (
				priority == i->priority &&
				i->position.w() > 0.5f &&
				distance < channelDistance
			)
			{
				if (i->handle)
					i->handle->detach();

				i->position = position.xyz1();
				i->surroundFilter = surroundFilter;
				i->lowPassFilter = lowPassFilter;
				i->echoFilter = echoFilter;
				i->sound = sound;
				i->soundChannel->play(
					sound->getBuffer(),
					sound->getCategory(),
					sound->getVolume(),
					presence,
					sound->getPresenceRate()
				);
				i->soundChannel->setFilter(groupFilter);
				i->soundChannel->setVolume(1.0f);
				i->priority = priority;
				i->time = time;
				i->fadeOff = -1.0f;
				i->handle = new SoundHandle(i->soundChannel, i->position, i->fadeOff);

				return i->handle;
			}
		}
	}

	return 0;
}

void SoundPlayer::setListenerTransform(const Transform& listenerTransform)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	if (m_surroundEnvironment)
		m_surroundEnvironment->setListenerTransform(listenerTransform);
}

Transform SoundPlayer::getListenerTransform() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	if (m_surroundEnvironment)
		return m_surroundEnvironment->getListenerTransform();
	else
		return Transform::identity();
}

void SoundPlayer::update(float dT)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (m_surroundEnvironment)
	{
		Vector4 listenerPosition = m_surroundEnvironment->getListenerTransform().translation().xyz1();

		// Update surround and low pass filters on playing 3d sounds.
		for (AlignedVector< Channel >::iterator i = m_channels.begin(); i != m_channels.end(); ++i)
		{
			// Skip non-playing or non-positional sounds.
			if (!i->soundChannel->isPlaying() || i->position.w() < 0.5f)
				continue;

			float maxDistance = i->sound->getRange();
			if (maxDistance <= m_surroundEnvironment->getInnerRadius())
				maxDistance = m_surroundEnvironment->getMaxDistance();

			// Calculate distance from listener; stop sounds which has moved outside max listener distance.
			Scalar distance = (i->position - listenerPosition).xyz0().length();
			if (distance > maxDistance)
			{
				if (i->handle)
					i->handle->detach();

				i->position = Vector4::zero();
				i->sound = 0;
				i->soundChannel->setFilter(0);
				i->soundChannel->stop();

				continue;
			}

			// Calculate cut-off frequency.
			float k0 = clamp< float >(distance / maxDistance, 0.0f, 1.0f);
			float cutOff = lerp(c_nearCutOff, c_farCutOff, std::sqrt(k0));

			// Set filter parameters.
			if (i->lowPassFilter)
				i->lowPassFilter->setCutOff(cutOff);
			if (i->surroundFilter)
				i->surroundFilter->setSpeakerPosition(i->position);

			// Set automatic sound parameters.
			i->soundChannel->setParameter(s_handleDistance, k0);
			i->soundChannel->setParameter(s_handleVelocity, 0.0f);

			// Disable repeat if no-one else then me have a reference to the handle.
			if (!i->handle || i->handle->getReferenceCount() <= 1)
				i->soundChannel->disableRepeat();
		}
	}

	// Update fade-off channels.
	for (AlignedVector< Channel >::iterator i = m_channels.begin(); i != m_channels.end(); ++i)
	{
		if (!i->soundChannel->isPlaying() || i->fadeOff <= 0.0f)
			continue;

		i->fadeOff -= std::min(dT, 1.0f / 60.0f);
		if (i->fadeOff > 0.0f)
			i->soundChannel->setVolume(i->fadeOff);
		else
			i->soundChannel->stop();
	}
}

	}
}
