/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <ctime>
#include "World/Entity/PointLightEntity.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PointLightEntity", PointLightEntity, Entity)

PointLightEntity::PointLightEntity(
	const Transform& transform,
	const Vector4& sunColor,
	const Vector4& baseColor,
	const Vector4& shadowColor,
	float range,
	float randomFlickerAmount,
	float randomFlickerFilter
)
:	m_transform(transform)
,	m_sunColor(sunColor)
,	m_baseColor(baseColor)
,	m_shadowColor(shadowColor)
,	m_range(range)
,	m_randomFlickerAmount(randomFlickerAmount)
,	m_randomFlickerFilter(randomFlickerFilter)
,	m_randomFlickerValue(0.0f)
,	m_randomFlicker(1.0f)
,	m_random(uint32_t(clock()))
{
}

void PointLightEntity::update(const UpdateParams& update)
{
	m_randomFlickerValue = m_random.nextFloat() * (1.0f - m_randomFlickerFilter) + m_randomFlickerValue * m_randomFlickerFilter;
	m_randomFlicker = 1.0f - m_randomFlickerValue * m_randomFlickerAmount;
}

void PointLightEntity::setTransform(const Transform& transform)
{
	m_transform = transform;
}

bool PointLightEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Aabb3 PointLightEntity::getBoundingBox() const
{
	return Aabb3(Vector4(-m_range, -m_range, -m_range, 1.0f), Vector4(m_range, m_range, m_range, 1.0f));
}

	}
}
