#include "World/Entity/Entity.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.Entity", Entity, Object)

Entity::~Entity()
{
	destroy();
}

void Entity::destroy()
{
	// Nothing to destroy.
}

void Entity::addReference(Entity* reference)
{
	// Do nothing.
}

void Entity::addReferee(Entity* referee)
{
	// Do nothing.
}

	}
}
