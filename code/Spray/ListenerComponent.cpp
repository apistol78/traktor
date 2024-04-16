/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/Player/ISoundListener.h"
#include "Sound/Player/ISoundPlayer.h"
#include "Spray/ListenerComponent.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.ListenerComponent", ListenerComponent, world::IEntityComponent)

ListenerComponent::ListenerComponent(sound::ISoundPlayer* soundPlayer)
:	m_soundPlayer(soundPlayer)
{
	if (m_soundPlayer != nullptr)
	{
		m_soundListener = m_soundPlayer->createListener();
		m_soundPlayer->addListener(m_soundListener);
	}
}

void ListenerComponent::destroy()
{
	if (m_soundPlayer != nullptr)
	{
		m_soundPlayer->removeListener(m_soundListener);
		m_soundPlayer = nullptr;
	}
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
	if (m_soundPlayer == nullptr || enable == isEnable())
		return;

	if (enable)
	{
		m_soundListener = m_soundPlayer->createListener();
		m_soundPlayer->addListener(m_soundListener);
	}
	else
	{
		m_soundPlayer->removeListener(m_soundListener);
		m_soundPlayer = nullptr;
	}
}

bool ListenerComponent::isEnable() const
{
	return m_soundListener != nullptr;
}

}
