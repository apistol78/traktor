/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/Sound.h"
#include "Sound/Player/ISoundHandle.h"
#include "Sound/Player/ISoundPlayer.h"
#include "Spray/SoundComponent.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SoundComponent", SoundComponent, world::IEntityComponent)

SoundComponent::SoundComponent(sound::ISoundPlayer* soundPlayer, const resource::Proxy< sound::Sound >& sound)
:	m_soundPlayer(soundPlayer)
,	m_sound(sound)
{
}

void SoundComponent::destroy()
{
	stop();
	m_sound.clear();
}

void SoundComponent::setOwner(world::Entity* owner)
{
}

void SoundComponent::setTransform(const Transform& transform)
{
	m_transform = transform;
	if (m_handle != nullptr)
		m_handle->setPosition(transform.translation().xyz1());
}

Aabb3 SoundComponent::getBoundingBox() const
{
	return Aabb3();
}

void SoundComponent::update(const world::UpdateParams& update)
{
}

void SoundComponent::play()
{
	if (m_handle != nullptr || m_soundPlayer == nullptr)
		return;

	m_handle = m_soundPlayer->play(m_sound, m_transform.translation().xyz1(), 32, false);
}

void SoundComponent::stop()
{
	if (m_handle == nullptr)
		return;

	m_handle->stop();
	m_handle = nullptr;
}

void SoundComponent::setVolume(float volume)
{
	if (m_handle != nullptr)
		m_handle->setVolume(volume);
}

void SoundComponent::setPitch(float pitch)
{
	if (m_handle != nullptr)
		m_handle->setPitch(pitch);
}

void SoundComponent::setParameter(int32_t id, float parameter)
{
	if (m_handle != nullptr)
		m_handle->setParameter(id, parameter);
}

}
