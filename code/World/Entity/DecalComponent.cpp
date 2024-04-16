/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/DecalComponent.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.DecalComponent", DecalComponent, IEntityComponent)

DecalComponent::DecalComponent(
	const Vector2& size,
	float thickness,
	float alpha,
	float cullDistance,
	const resource::Proxy< render::Shader >& shader
)
:	m_size(size)
,	m_thickness(thickness)
,	m_alpha(alpha)
,	m_cullDistance(cullDistance)
,	m_age(0.0f)
,	m_shader(shader)
{
}

void DecalComponent::destroy()
{
}

void DecalComponent::setOwner(Entity* owner)
{
}

void DecalComponent::setTransform(const Transform& transform)
{
	m_transform = transform;
}

Aabb3 DecalComponent::getBoundingBox() const
{
	return Aabb3(
		Vector4(-m_size.x, -m_thickness, -m_size.y, 1.0f),
		Vector4(m_size.x, m_thickness, m_size.y, 1.0f)
	);
}

void DecalComponent::update(const UpdateParams& update)
{
	m_age += (float)update.deltaTime;
}

}
