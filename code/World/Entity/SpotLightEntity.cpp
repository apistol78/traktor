#include "World/Entity/SpotLightEntity.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.SpotLightEntity", SpotLightEntity, SpatialEntity)

SpotLightEntity::SpotLightEntity(
	const Transform& transform,
	const Vector4& sunColor,
	const Vector4& baseColor,
	const Vector4& shadowColor,
	float range,
	float radius
)
:	m_transform(transform)
,	m_sunColor(sunColor)
,	m_baseColor(baseColor)
,	m_shadowColor(shadowColor)
,	m_range(range)
,	m_radius(radius)
{
}

void SpotLightEntity::update(const EntityUpdate* update)
{
}

void SpotLightEntity::setTransform(const Transform& transform)
{
	m_transform = transform;
}

bool SpotLightEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Aabb3 SpotLightEntity::getBoundingBox() const
{
	return Aabb3(Vector4(-m_range, -m_range, -m_range, 1.0f), Vector4(m_range, m_range, m_range, 1.0f));
}

	}
}
