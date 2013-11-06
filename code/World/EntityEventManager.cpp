#include "World/EntityEventManager.h"
#include "World/EntityEventSet.h"
#include "World/IEntityEvent.h"
#include "World/IEntityEventInstance.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityEventManager", EntityEventManager, IEntityEventManager)

EntityEventManager::EntityEventManager(uint32_t maxEventInstances)
:	m_maxEventInstances(maxEventInstances)
{
	m_eventInstances.reserve(maxEventInstances);
}

IEntityEventInstance* EntityEventManager::raise(const IEntityEvent* event, Entity* sender, const Transform& Toffset)
{
	if (!event || m_eventInstances.size() >= m_maxEventInstances)
		return 0;

	Ref< IEntityEventInstance > eventInstance = event->createInstance(this, sender, Toffset);
	if (eventInstance)
		m_eventInstances.push_back(eventInstance);

	return eventInstance;
}

IEntityEventInstance* EntityEventManager::raise(const EntityEventSet* eventSet, const std::wstring& eventId, Entity* sender, const Transform& Toffset)
{
	if (!eventSet || eventId.empty())
		return 0;

	const IEntityEvent* event = eventSet->getEvent(eventId);
	if (!event)
		return 0;

	return raise(event, sender, Toffset);
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
