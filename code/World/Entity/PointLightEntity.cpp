#include "World/Entity/PointLightEntity.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PointLightEntity", PointLightEntity, SpatialEntity)

PointLightEntity::PointLightEntity(
	const Transform& transform,
	const Vector4& sunColor,
	const Vector4& baseColor,
	const Vector4& shadowColor,
	float range
)
:	m_transform(transform)
,	m_sunColor(sunColor)
,	m_baseColor(baseColor)
,	m_shadowColor(shadowColor)
,	m_range(range)
{
}

void PointLightEntity::update(const EntityUpdate* update)
{
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

Aabb PointLightEntity::getBoundingBox() const
{
	return Aabb(Vector4(-m_range, -m_range, -m_range, 1.0f), Vector4(m_range, m_range, m_range, 1.0f));
}

	}
}
