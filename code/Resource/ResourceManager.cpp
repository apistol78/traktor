#include <algorithm>
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Timer/Timer.h"
#include "Resource/CachedResourceHandle.h"
#include "Resource/IResourceFactory.h"
#include "Resource/ResourceManager.h"
#include "Resource/UncachedResourceHandle.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.resource.ResourceManager", ResourceManager, IResourceManager)

ResourceManager::~ResourceManager()
{
	destroy();
}

void ResourceManager::destroy()
{
	m_factories.clear();
	m_cachedHandles.clear();
	m_uncachedHandles.clear();
	m_times.clear();
}

void ResourceManager::addFactory(IResourceFactory* factory)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_factories.push_back(factory);
}

void ResourceManager::removeFactory(IResourceFactory* factory)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_factories.remove(factory);
}

void ResourceManager::removeAllFactories()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_factories.clear();
}

Ref< IResourceHandle > ResourceManager::bind(const TypeInfo& type, const Guid& guid)
{
	Ref< IResourceHandle > handle;

	if (guid.isNull() || !guid.isValid())
		return 0;

	Ref< IResourceFactory > factory = findFactory(type);
	if (!factory)
		return 0;

	bool cacheable = factory->isCacheable();
	if (cacheable)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		std::map< Guid, Ref< CachedResourceHandle > >::iterator i = m_cachedHandles.find(guid);
		if (i != m_cachedHandles.end())
			handle = i->second;
		else
		{
			Ref< CachedResourceHandle > cachedHandle = new CachedResourceHandle(type);
			m_cachedHandles[guid] = cachedHandle;
			handle = cachedHandle;
		}
	}
	else
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		RefArray< UncachedResourceHandle >& handles = m_uncachedHandles[guid];

		// First try to reuse handles which are no longer in use; as final proxy reference is released resource
		// handles becomes tagged as not being in se and thus it's safe to reuse them.
		for (RefArray< UncachedResourceHandle >::iterator i = handles.begin(); i != handles.end(); ++i)
		{
			if (!(*i)->inUse())
			{
				handle = *i;
				break;
			}
		}

		if (!handle)
		{
			Ref< UncachedResourceHandle > uncachedHandle = new UncachedResourceHandle(type);
			handles.push_back(uncachedHandle);
			handle = uncachedHandle;
		}
	}
	
	T_ASSERT (handle);

	if (!handle->get())
	{
		load(guid, factory, type, handle);
		if (!handle->get())
			return 0;
	}

	return handle;
}

void ResourceManager::reload(const Guid& guid)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	std::map< Guid, RefArray< UncachedResourceHandle > >::iterator i0 = m_uncachedHandles.find(guid);
	if (i0 != m_uncachedHandles.end())
	{
		const RefArray< UncachedResourceHandle >& handles = i0->second;
		for (RefArray< UncachedResourceHandle >::const_iterator i = handles.begin(); i != handles.end(); ++i)
		{
			const TypeInfo& resourceType = (*i)->getResourceType();
			Ref< IResourceFactory > factory = findFactory(resourceType);
			if (factory)
				load(guid, factory, resourceType, *i);
		}
		return;
	}

	std::map< Guid, Ref< CachedResourceHandle > >::iterator i1 = m_cachedHandles.find(guid);
	if (i1 != m_cachedHandles.end())
	{
		const TypeInfo& resourceType = i1->second->getResourceType();
		Ref< IResourceFactory > factory = findFactory(resourceType);
		if (factory)
			load(guid, factory, resourceType, i1->second);
		return;
	}
}

void ResourceManager::flush(const Guid& guid)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	std::map< Guid, RefArray< UncachedResourceHandle > >::iterator i0 = m_uncachedHandles.find(guid);
	if (i0 != m_uncachedHandles.end())
	{
		const RefArray< UncachedResourceHandle >& handles = i0->second;
		for (RefArray< UncachedResourceHandle >::const_iterator i = handles.begin(); i != handles.end(); ++i)
			(*i)->flush();
		return;
	}

	std::map< Guid, Ref< CachedResourceHandle > >::iterator i1 = m_cachedHandles.find(guid);
	if (i1 != m_cachedHandles.end())
	{
		i1->second->flush();
		return;
	}
}

void ResourceManager::flush()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	for (std::map< Guid, RefArray< UncachedResourceHandle > >::iterator i = m_uncachedHandles.begin(); i != m_uncachedHandles.end(); ++i)
	{
		for (RefArray< UncachedResourceHandle >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
			(*j)->flush();
	}

	for (std::map< Guid, Ref< CachedResourceHandle > >::iterator i = m_cachedHandles.begin(); i != m_cachedHandles.end(); ++i)
		i->second->flush();
}

void ResourceManager::dumpStatistics()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	log::debug << L"Resource manager statistics:" << Endl;
	log::debug << IncreaseIndent;
	
	log::debug << uint32_t(m_cachedHandles.size()) << L" persistent entries" << Endl;
	log::debug << uint32_t(m_uncachedHandles.size()) << L" transient entries" << Endl;
	
	double totalTime = 0.0;
	for (std::map< const TypeInfo*, TimeCount >::const_iterator i = m_times.begin(); i != m_times.end(); ++i)
		totalTime += i->second.time;
	for (std::map< const TypeInfo*, TimeCount >::const_iterator i = m_times.begin(); i != m_times.end(); ++i)
		log::debug << i->first->getName() << L" " << (i->second.time * 1000.0) << L" ms (" << i->second.count << L" resource(s), " << (i->second.time * 100.0f / totalTime) << L"%)" << Endl;
	
	log::debug << DecreaseIndent;
}

Ref< IResourceFactory > ResourceManager::findFactory(const TypeInfo& type)
{
	for (RefArray< IResourceFactory >::iterator i = m_factories.begin(); i != m_factories.end(); ++i)
	{
		TypeInfoSet typeSet = (*i)->getResourceTypes();
		if (std::find(typeSet.begin(), typeSet.end(), &type) != typeSet.end())
			return *i;
	}
	return 0;
}

void ResourceManager::load(const Guid& guid, IResourceFactory* factory, const TypeInfo& resourceType, IResourceHandle* handle)
{
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();

	// Do not attempt to load resource if thread has been stopped; application probably terminating
	// so we try to leave early.
	if (currentThread->stopped())
	{
		log::info << L"Resource loader thread stopped; skipped loading resource" << Endl;
		return;
	}

	m_timeStack.push(0.0);

	Timer timer;
	timer.start();

	//const TypeInfo& resourceType = handle->getResourceType();
	Ref< Object > object = factory->create(this, resourceType, guid);
	if (object)
	{
		T_ASSERT_M (is_type_of(resourceType, type_of(object)), L"Incorrect type of created resource");
		
		log::info << L"Resource \"" << guid.format() << L"\" (" << type_name(object) << L") created" << Endl;
		handle->replace(object);

		// Yield current thread; we want other threads to get some periodic CPU time to
		// render loading screens etc. Use sleep as we want lower priority threads also
		// to be able to run.
		currentThread->sleep(0);
	}
	else
		log::error << L"Unable to create resource \"" << guid.format() << L"\" (" << resourceType.getName() << L")" << Endl;

	// Accumulate time spend on creating resources.
	double time = timer.getElapsedTime();

	m_times[&resourceType].count++;
	m_times[&resourceType].time += time + m_timeStack.top();
	m_timeStack.pop();

	if (!m_timeStack.empty())
		m_timeStack.top() -= time;
}

	}
}
