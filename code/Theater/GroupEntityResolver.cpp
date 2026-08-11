/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/GroupEntityResolver.h"

#include "World/Entity.h"
#include "World/Entity/GroupComponent.h"

namespace traktor::theater
{
namespace
{

world::Entity* findEntityInGroup(const world::GroupComponent* group, const Guid& id)
{
	for (auto entity : group->getEntities())
	{
		if (entity->getId() == id)
			return entity;
	}

	// Not a direct child of the group; continue search in nested groups.
	for (auto entity : group->getEntities())
	{
		if (auto childGroup = entity->getComponent< world::GroupComponent >())
		{
			if (world::Entity* foundEntity = findEntityInGroup(childGroup, id))
				return foundEntity;
		}
	}

	return nullptr;
}

}

GroupEntityResolver::GroupEntityResolver(const world::GroupComponent* group)
:	m_group(group)
{
}

world::Entity* GroupEntityResolver::findEntity(const Guid& id) const
{
	return m_group != nullptr ? findEntityInGroup(m_group, id) : nullptr;
}

}
