/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Vector4.h"
#include "Sound/AudioChannel.h"
#include "Sound/Player/SoundHandle.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundHandle", SoundHandle, ISoundHandle)

void SoundHandle::stop()
{
	if (m_channel)
		m_channel->stop();

	detach();
}

void SoundHandle::fadeOff()
{
	if (m_fadeOff)
		*m_fadeOff = 1.0f;

	detach();
}

bool SoundHandle::isPlaying()
{
	return m_channel ? m_channel->isPlaying() : false;
}

void SoundHandle::setVolume(float volume)
{
	if (m_channel)
		m_channel->setVolume(volume);
}

void SoundHandle::setPitch(float pitch)
{
	if (m_channel)
		m_channel->setPitch(pitch);
}

void SoundHandle::setPosition(const Vector4& position)
{
	if (m_position)
		*m_position = position.xyz1();
}

void SoundHandle::setParameter(int32_t id, float parameter)
{
	if (m_channel)
		m_channel->setParameter(id, parameter);
}

SoundHandle::SoundHandle(AudioChannel* channel, Vector4& position, float& fadeOff)
:	m_channel(channel)
,	m_position(&position)
,	m_fadeOff(&fadeOff)
{
}

void SoundHandle::detach()
{
	m_channel = nullptr;
	m_position = 0;
	m_fadeOff = 0;
}

	}
}
