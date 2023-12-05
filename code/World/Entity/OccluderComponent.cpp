/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity.h"
#include "World/Entity/OccluderComponent.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.OccluderComponent", OccluderComponent, IEntityComponent)

OccluderComponent::OccluderComponent(const Aabb3& volume)
:	m_volume(volume)
{
}

void OccluderComponent::destroy()
{
}

void OccluderComponent::setOwner(Entity* owner)
{
}

void OccluderComponent::setTransform(const Transform& transform)
{
}

Aabb3 OccluderComponent::getBoundingBox() const
{
	return m_volume;
}

void OccluderComponent::update(const UpdateParams& update)
{
}

}
