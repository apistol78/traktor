#include "World/Entity/NullEntity.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.NullEntity", NullEntity, SpatialEntity)

NullEntity::NullEntity(const Transform& transform)
{
	m_transform[0] =
	m_transform[1] = transform;
}

void NullEntity::update(const EntityUpdate* update)
{
}

void NullEntity::setTransform(const Transform& transform)
{
	m_transform[0] = m_transform[1];
	m_transform[1] = transform;
}

bool NullEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform[1];
	return true;
}

Aabb3 NullEntity::getBoundingBox() const
{
	return Aabb3(
		Vector4::zero(),
		Vector4::zero()
	);
}

Transform NullEntity::getTransform(float interval) const
{
	return lerp(m_transform[0], m_transform[1], Scalar(interval));
}

	}
}
