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

Aabb SpatialEntity::getWorldBoundingBox() const
{
	Aabb boundingBox = getBoundingBox();
	if (boundingBox.empty())
		return Aabb();

	Transform transform;
	return getTransform(transform) ? boundingBox.transform(transform) : boundingBox;
}

	}
}
