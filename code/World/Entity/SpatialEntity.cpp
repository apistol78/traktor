#include "World/Entity/SpatialEntity.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.SpatialEntity", SpatialEntity, Entity)

void SpatialEntity::setTransform(const Transform& transform)
{
	// Don't do anything.
}

bool SpatialEntity::getTransform(Transform& outTransform) const
{
	// Doesn't have a transform.
	return false;
}

Aabb3 SpatialEntity::getWorldBoundingBox() const
{
	Aabb3 boundingBox = getBoundingBox();
	if (boundingBox.empty())
		return Aabb3();

	Transform transform;
	return getTransform(transform) ? boundingBox.transform(transform) : boundingBox;
}

	}
}
