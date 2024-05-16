/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity.h"
#include "World/Entity/DecalEvent.h"
#include "World/Entity/DecalEventInstance.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.DecalEvent", DecalEvent, IEntityEvent)

Ref< IEntityEventInstance > DecalEvent::createInstance(EventManagerComponent* eventManager, Entity* sender, const Transform& Toffset) const
{
	const Transform T = sender->getTransform() * Toffset;
	return new DecalEventInstance(this, sender->getWorld(), T);
}

}
