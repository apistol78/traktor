/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/WorldEntityResolver.h"

#include "World/World.h"

namespace traktor::theater
{

WorldEntityResolver::WorldEntityResolver(const world::World* world)
:	m_world(world)
{
}

world::Entity* WorldEntityResolver::findEntity(const Guid& id) const
{
	return m_world != nullptr ? m_world->getEntity(id) : nullptr;
}

}
