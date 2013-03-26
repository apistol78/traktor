#include "World/EntityEventManager.h"
#include "World/IEntityEvent.h"
#include "World/IEntityEventInstance.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

const uint32_t c_maxEventInstances = 16;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityEventManager", EntityEventManager, IEntityEventManager)

void EntityEventManager::raise(const IEntityEvent* event, Entity* sender, const Transform& Toffset)
{
	if (!event || m_eventInstances.size() >= c_maxEventInstances)
		return;

	Ref< IEntityEventInstance > eventInstance = event->createInstance(sender, Toffset);
	if (eventInstance)
		m_eventInstances.push_back(eventInstance);
}

void EntityEventManager::update(const UpdateParams& update)
{
	for (RefArray< IEntityEventInstance >::iterator i = m_eventInstances.begin(); i != m_eventInstances.end(); )
	{
		if ((*i)->update(update))
			++i;
		else
			i = m_eventInstances.erase(i);
	}
}

void EntityEventManager::build(IWorldRenderer* worldRenderer)
{
	for (RefArray< IEntityEventInstance >::iterator i = m_eventInstances.begin(); i != m_eventInstances.end(); ++i)
		(*i)->build(worldRenderer);
}

	}
}
