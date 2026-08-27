/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/DisplacementEntityComponent.h"

#include "World/Entity.h"
#include "World/Entity/DisplacementEntityComponentData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.DisplacementEntityComponent", DisplacementEntityComponent, IEntityComponent)

DisplacementEntityComponent::DisplacementEntityComponent(const DisplacementEntityComponentData* data)
	: m_radius(data->getRadius())
	, m_strength(data->getStrength())
	, m_falloff(data->getFalloff())
	, m_press(data->getPress())
{
}

void DisplacementEntityComponent::destroy()
{
	m_owner = nullptr;
}

void DisplacementEntityComponent::setOwner(Entity* owner)
{
	if ((m_owner = owner) != nullptr)
		m_position = m_owner->getTransform().translation().xyz1();
}

void DisplacementEntityComponent::setTransform(const Transform& transform)
{
	m_position = transform.translation().xyz1();
}

Aabb3 DisplacementEntityComponent::getBoundingBox() const
{
	const Scalar radius(m_radius);
	return Aabb3(Vector4(-radius, -radius, -radius, 1.0_simd), Vector4(radius, radius, radius, 1.0_simd));
}

void DisplacementEntityComponent::update(const UpdateParams& update)
{
}

}
