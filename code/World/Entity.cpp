#include "World/Entity.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.Entity", Entity, Object)

void Entity::destroy()
{
	// Nothing to destroy.
}

void Entity::setTransform(const Transform& transform)
{
	// Don't do anything.
}

Transform Entity::getTransform() const
{
	// Doesn't have a transform.
	return Transform::identity();
}

	}
}
