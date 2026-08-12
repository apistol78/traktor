/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/Player/SoundListener.h"
#include "Sound/Player/SoundPlayer.h"
#include "Spray/ListenerComponent.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.ListenerComponent", ListenerComponent, world::IEntityComponent)

ListenerComponent::ListenerComponent(sound::SoundPlayer* soundPlayer)
:	m_soundPlayer(soundPlayer)
{
	if (m_soundPlayer != nullptr)
	{
		m_soundListener = new sound::SoundListener();
		m_soundPlayer->addListener(m_soundListener);
		m_enabled = true;
	}
}

void ListenerComponent::destroy()
{
	// Unregister before letting go of the player; a listener left behind keep being
	// mixed against, frozen wherever it last stood, and there is no way back to it.
	setEnable(false);
	m_soundListener = nullptr;
	m_soundPlayer = nullptr;
}

void ListenerComponent::setOwner(world::Entity* owner)
{
}

void ListenerComponent::setTransform(const Transform& transform)
{
	if (m_soundListener != nullptr)
		m_soundListener->setTransform(transform);
}

Aabb3 ListenerComponent::getBoundingBox() const
{
	return Aabb3();
}

void ListenerComponent::update(const world::UpdateParams& update)
{
}

void ListenerComponent::setEnable(bool enable)
{
	if (m_soundPlayer == nullptr || m_soundListener == nullptr || enable == m_enabled)
		return;

	// Only the registration with the player is toggled; the listener itself is kept so
	// it holds on to its transform and re-enabling does not spatialize a frame against
	// the origin before the owner next moves.
	if (enable)
		m_soundPlayer->addListener(m_soundListener);
	else
		m_soundPlayer->removeListener(m_soundListener);

	m_enabled = enable;
}

bool ListenerComponent::isEnable() const
{
	return m_enabled;
}

}
