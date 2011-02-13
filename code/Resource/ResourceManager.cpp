#include <algorithm>
#include "Resource/ResourceManager.h"
#include "Resource/ResourceHandle.h"
#include "Resource/IResourceFactory.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/Acquire.h"
#include "Core/Timer/Timer.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace resource
	{
		namespace
		{

class NullResourceHandle : public IResourceHandle
{
public:
	virtual void replace(Object* object) { T_BREAKPOINT; }

	virtual Object* get() const { return 0; }

	virtual void flush() {}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.resource.ResourceManager", ResourceManager, IResourceManager)

ResourceManager::ResourceManager()
:	m_nullHandle(new NullResourceHandle())
{
}

ResourceManager::~ResourceManager()
{
	destroy();
}

void ResourceManager::destroy()
{
	m_factories.clear();
	m_nullHandle = 0;
	m_cache.clear();
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
	Ref< ResourceHandle > handle;

	if (guid.isNull() || !guid.isValid())
		return m_nullHandle;

	Ref< IResourceFactory > factory = findFactory(type);
	if (!factory)
		return m_nullHandle;

	bool cacheable = factory->isCacheable();

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		RefArray< ResourceHandle >& cache = m_cache[guid];
		if (cacheable && !cache.empty())
			handle = cache.front();
		else
		{
			// First try to reuse empty handles; as final proxy reference is released resource
			// handles becomes empty and thus it's safe to reuse them.
			for (RefArray< ResourceHandle >::iterator i = cache.begin(); i != cache.end(); ++i)
			{
				if (!(*i)->get())
				{
					handle = *i;
					break;
				}
			}
			if (!handle)
			{
				handle = new ResourceHandle(type, cacheable);
				cache.push_back(handle);
			}
		}
	}
	
	T_ASSERT (handle);

	if (!handle->get())
		load(guid, factory, handle);

	return handle;
}

void ResourceManager::update(const Guid& guid, bool force)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	std::map< Guid, RefArray< ResourceHandle > >::iterator i = m_cache.find(guid);
	if (i == m_cache.end())
		return;

	const RefArray< ResourceHandle >& handles = i->second;
	for (RefArray< ResourceHandle >::const_iterator i = handles.begin(); i != handles.end(); ++i)
	{
		if (!force && (*i)->get())
			continue;

		Ref< IResourceFactory > factory = findFactory((*i)->getResourceType());
		if (factory)
			load(guid, factory, *i);
	}
}

void ResourceManager::flush(const Guid& guid)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	std::map< Guid, RefArray< ResourceHandle > >::iterator i = m_cache.find(guid);
	if (i == m_cache.end())
		return;

	const RefArray< ResourceHandle >& handles = i->second;
	for (RefArray< ResourceHandle >::const_iterator i = handles.begin(); i != handles.end(); ++i)
		(*i)->flush();
}

void ResourceManager::flush()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (std::map< Guid, RefArray< ResourceHandle > >::iterator i = m_cache.begin(); i != m_cache.end(); ++i)
	{
		for (RefArray< ResourceHandle >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
			(*j)->flush();
	}
}

void ResourceManager::dumpStatistics()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	log::debug << L"Resource manager statistics:" << Endl;
	log::debug << IncreaseIndent;
	
	log::debug << uint32_t(m_cache.size()) << L" cache entries" << Endl;
	
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

void ResourceManager::load(const Guid& guid, IResourceFactory* factory, ResourceHandle* handle)
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

	const TypeInfo& resourceType = handle->getResourceType();
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
