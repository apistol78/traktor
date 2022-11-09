#include "Core/Thread/Acquire.h"
#include "World/EntityEventManager.h"
#include "World/IEntityEvent.h"
#include "World/IEntityEventInstance.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityEventManager", EntityEventManager, Object)

EntityEventManager::EntityEventManager(uint32_t maxEventInstances)
:	m_maxEventInstances(maxEventInstances)
{
	m_eventInstances.reserve(maxEventInstances);
}

IEntityEventInstance* EntityEventManager::raise(const IEntityEvent* event, Entity* sender, const Transform& Toffset)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!event || m_eventInstances.size() >= m_maxEventInstances)
		return nullptr;

	Ref< IEntityEventInstance > eventInstance = event->createInstance(this, sender, Toffset);
	if (eventInstance)
		m_eventInstances.push_back(eventInstance);

	return eventInstance;
}

void EntityEventManager::update(const UpdateParams& update)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (auto it = m_eventInstances.begin(); it != m_eventInstances.end(); )
	{
		if ((*it)->update(update))
			++it;
		else
			it = m_eventInstances.erase(it);
	}
}

void EntityEventManager::gather(const std::function< void(Entity*) >& fn) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (auto eventInstance : m_eventInstances)
		eventInstance->gather(fn);
}

void EntityEventManager::cancelAll(Cancel when)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	// Issue cancel on all instances.
	for (auto eventInstance : m_eventInstances)
		eventInstance->cancel(when);

	// Remove all instances directly instead of waiting for next update.
	if (when == Cancel::Immediate)
		m_eventInstances.resize(0);
}

}
