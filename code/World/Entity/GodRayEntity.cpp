#include "World/Entity/GodRayEntity.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.GodRayEntity", GodRayEntity, Entity)

GodRayEntity::GodRayEntity(const Transform& transform)
:	m_transform(transform)
{
}

void GodRayEntity::update(const UpdateParams& update)
{
}

void GodRayEntity::setTransform(const Transform& transform)
{
	m_transform = transform;
}

bool GodRayEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Aabb3 GodRayEntity::getBoundingBox() const
{
	return Aabb3();
}

	}
}
