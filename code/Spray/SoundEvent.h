/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Resource/Proxy.h"
#include "World/IEntityEvent.h"

namespace traktor::sound
{

class ISoundPlayer;
class Sound;

}

namespace traktor::spray
{

/*! Sound event.
 * \ingroup Spray
 */
class SoundEvent : public world::IEntityEvent
{
	T_RTTI_CLASS;

public:
	explicit SoundEvent(
		sound::ISoundPlayer* soundPlayer,
		const resource::Proxy< sound::Sound >& sound,
		bool positional,
		bool follow,
		bool autoStopFar
	);

	virtual Ref< world::IEntityEventInstance > createInstance(world::EventManagerComponent* eventManager, world::Entity* sender, const Transform& Toffset) const override final;

private:
	sound::ISoundPlayer* m_soundPlayer;
	resource::Proxy< sound::Sound > m_sound;
	bool m_positional;
	bool m_follow;
	bool m_autoStopFar;
};

}
