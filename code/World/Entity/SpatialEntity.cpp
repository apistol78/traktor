#include "World/Entity/SpatialEntity.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.SpatialEntity", SpatialEntity, Entity)

void SpatialEntity::setTransform(const Matrix44& transform)
{
	// Don't do anything.
}

bool SpatialEntity::getTransform(Matrix44& outTransform) const
{
	// Doesn't have a transform.
	return false;
}

Aabb SpatialEntity::getWorldBoundingBox() const
{
	Aabb boundingBox = getBoundingBox();
	if (boundingBox.empty())
		return Aabb();

	Matrix44 transform;
	return getTransform(transform) ? boundingBox.transform(transform) : boundingBox;
}

	}
}
