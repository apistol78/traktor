#include "World/Entity/EventSetComponent.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EventSetComponent", EventSetComponent, IEntityComponent)

void EventSetComponent::destroy()
{
}

void EventSetComponent::setOwner(Entity* owner)
{
}

void EventSetComponent::setTransform(const Transform& transform)
{
}

Aabb3 EventSetComponent::getBoundingBox() const
{
	return Aabb3();
}

void EventSetComponent::update(const UpdateParams& update)
{
}

world::IEntityEvent* EventSetComponent::getEvent(const std::wstring& name) const
{
	auto it = m_events.find(name);
	return it != m_events.end() ? it->second : nullptr;
}

	}
}
