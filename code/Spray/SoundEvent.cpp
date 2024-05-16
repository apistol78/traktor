/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/SoundEvent.h"
#include "Spray/SoundEventInstance.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SoundEvent", SoundEvent, world::IEntityEvent)

SoundEvent::SoundEvent(
	sound::ISoundPlayer* soundPlayer,
	const resource::Proxy< sound::Sound >& sound,
	bool positional,
	bool follow,
	bool autoStopFar
)
:	m_soundPlayer(soundPlayer)
,	m_sound(sound)
,	m_positional(positional)
,	m_follow(follow)
,	m_autoStopFar(autoStopFar)
{
}

Ref< world::IEntityEventInstance > SoundEvent::createInstance(world::EventManagerComponent* eventManager, world::Entity* sender, const Transform& Toffset) const
{
	if (m_soundPlayer)
		return new SoundEventInstance(sender, Toffset, m_soundPlayer, m_sound, m_positional, m_follow, m_autoStopFar);
	else
		return nullptr;
}

}
