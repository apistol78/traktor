/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity.h"
#include "World/Entity/PersistentIdComponent.h"
#include "World/Entity/PersistentIdComponentData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PersistentIdComponent", PersistentIdComponent, IEntityComponent)

PersistentIdComponent::PersistentIdComponent(const Guid& id)
:	m_id(id)
{
}

void PersistentIdComponent::destroy()
{
}

void PersistentIdComponent::setOwner(Entity* owner)
{
}

void PersistentIdComponent::setTransform(const Transform& transform)
{
}

Aabb3 PersistentIdComponent::getBoundingBox() const
{
	return Aabb3();
}

void PersistentIdComponent::update(const UpdateParams& update)
{
}

}
