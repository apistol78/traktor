/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Sound/Sound.h"
#include "Spray/SoundComponent.h"
#include "Spray/SoundComponentData.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spray.SoundComponentData", 0, SoundComponentData, world::IEntityComponentData)

SoundComponentData::SoundComponentData(const resource::Id< sound::Sound >& sound)
:	m_sound(sound)
{
}

Ref< SoundComponent > SoundComponentData::createComponent(resource::IResourceManager* resourceManager, sound::ISoundPlayer* soundPlayer) const
{
	resource::Proxy< sound::Sound > sound;
	if (!resourceManager->bind(m_sound, sound))
		return nullptr;

	return new SoundComponent(
		soundPlayer,
		sound
	);
}

int32_t SoundComponentData::getOrdinal() const
{
	return 0;
}

void SoundComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void SoundComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< sound::Sound >(L"sound", m_sound);
}

}
