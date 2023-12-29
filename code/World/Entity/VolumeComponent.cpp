/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity.h"
#include "World/Entity/VolumeComponent.h"
#include "World/Entity/VolumeComponentData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.VolumeComponent", VolumeComponent, IEntityComponent)

VolumeComponent::VolumeComponent(const VolumeComponentData* data)
:	m_owner(nullptr)
,	m_data(data)
{
	m_boundingBox = m_data->getBoundingBox();
}

void VolumeComponent::destroy()
{
}

void VolumeComponent::setOwner(Entity* owner)
{
	m_owner = owner;
}

void VolumeComponent::setTransform(const Transform& transform)
{
}

Aabb3 VolumeComponent::getBoundingBox() const
{
	return m_boundingBox;
}

void VolumeComponent::update(const UpdateParams& update)
{
}

bool VolumeComponent::inside(const Vector4& point) const
{
	if (!m_owner)
		return false;

	const Transform transform = m_owner->getTransform();
	const Vector4 p = transform.inverse() * point.xyz1();
	return m_boundingBox.inside(p);
}

}
