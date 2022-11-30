/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Random.h"
#include "World/Entity.h"
#include "World/Entity/LightComponent.h"

namespace traktor::world
{
	namespace
	{

Random s_random;
		
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.LightComponent", LightComponent, IEntityComponent)

LightComponent::LightComponent(
	LightType lightType,
	const Vector4& color,
	bool castShadow,
	float range,
	float radius,
	float flickerAmount,
	float flickerFilter
)
:	m_owner(nullptr)
,	m_lightType(lightType)
,	m_color(color)
,	m_castShadow(castShadow)
,	m_range(range)
,	m_radius(radius)
,	m_flickerAmount(flickerAmount)
,	m_flickerFilter(flickerFilter)
,	m_flickerValue(0.0f)
,	m_flickerCoeff(0.0f)
{
}

void LightComponent::destroy()
{
}

void LightComponent::setOwner(Entity* owner)
{
	m_owner = owner;
}

void LightComponent::update(const UpdateParams& update)
{
	m_flickerValue = s_random.nextFloat() * (1.0f - m_flickerFilter) + m_flickerValue * m_flickerFilter;
	m_flickerCoeff = 1.0f - m_flickerValue * m_flickerAmount;
}

void LightComponent::setTransform(const Transform& transform)
{
}

Aabb3 LightComponent::getBoundingBox() const
{
	return Aabb3();
}

Transform LightComponent::getTransform() const
{
	return m_owner->getTransform();
}

}
