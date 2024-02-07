/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Transform.h"
#include "Resource/Proxy.h"
#include "World/IEntityEventInstance.h"

namespace traktor::sound
{

class ISoundHandle;
class ISoundPlayer;
class Sound;

}

namespace traktor::world
{

class Entity;

}

namespace traktor::spray
{

/*! Sound event instance.
 * \ingroup Spray
 */
class SoundEventInstance : public world::IEntityEventInstance
{
	T_RTTI_CLASS;

public:
	explicit SoundEventInstance(
		world::Entity* sender,
		const Transform& Toffset,
		sound::ISoundPlayer* soundPlayer,
		const resource::Proxy< sound::Sound >& sound,
		bool positional,
		bool follow,
		bool autoStopFar
	);

	virtual bool update(const world::UpdateParams& update) override final;

	virtual void cancel(world::Cancel when) override final;

private:
	Ref< world::Entity > m_sender;
	Transform m_Toffset;
	sound::ISoundPlayer* m_soundPlayer;
	resource::Proxy< sound::Sound > m_sound;
	bool m_positional;
	bool m_follow;
	bool m_autoStopFar;
	Ref< sound::ISoundHandle > m_handle;
};

}
