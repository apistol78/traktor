#include "World/Entity.h"
#include "World/Entity/GodRayComponent.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.GodRayComponent", GodRayComponent, IEntityComponent)

GodRayComponent::GodRayComponent(Entity* owner)
:	m_owner(owner)
{
}

void GodRayComponent::destroy()
{
}

void GodRayComponent::update(const UpdateParams& update)
{
}

void GodRayComponent::setTransform(const Transform& transform)
{
}

Aabb3 GodRayComponent::getBoundingBox() const
{
	return Aabb3();
}

Transform GodRayComponent::getTransform() const
{
	Transform transform;
	m_owner->getTransform(transform);
	return transform;
}

	}
}
