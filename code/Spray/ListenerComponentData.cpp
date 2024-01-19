/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/ListenerComponent.h"
#include "Spray/ListenerComponentData.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spray.ListenerComponentData", 0, ListenerComponentData, world::IEntityComponentData)

Ref< ListenerComponent > ListenerComponentData::createComponent(sound::ISoundPlayer* soundPlayer) const
{
	return new ListenerComponent(soundPlayer);
}

int32_t ListenerComponentData::getOrdinal() const
{
	return 0;
}

void ListenerComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void ListenerComponentData::serialize(ISerializer& s)
{
}

}
