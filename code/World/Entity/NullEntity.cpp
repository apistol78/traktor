#include "World/Entity/NullEntity.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.NullEntity", NullEntity, SpatialEntity)

NullEntity::NullEntity(const Transform& transform)
:	m_transform(transform)
{
}

void NullEntity::update(const EntityUpdate* update)
{
	m_transform.step();
}

void NullEntity::setTransform(const Transform& transform)
{
	m_transform.set(transform);
}

bool NullEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform.get();
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
	return m_transform.get(interval);
}

	}
}
