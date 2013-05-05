#include <algorithm>
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Timer/Timer.h"
#include "Resource/ExclusiveResourceHandle.h"
#include "Resource/IResourceFactory.h"
#include "Resource/ResourceBundle.h"
#include "Resource/ResourceManager.h"
#include "Resource/ResidentResourceHandle.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.resource.ResourceManager", ResourceManager, IResourceManager)

ResourceManager::ResourceManager(bool verbose)
:	m_verbose(verbose)
{
}

ResourceManager::~ResourceManager()
{
	destroy();
}

void ResourceManager::destroy()
{
	m_factories.clear();
	m_residentHandles.clear();
	m_exclusiveHandles.clear();
	m_times.clear();
}

void ResourceManager::addFactory(const IResourceFactory* factory)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_factories.push_back(factory);
}

void ResourceManager::removeFactory(const IResourceFactory* factory)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_factories.remove(factory);
}

void ResourceManager::removeAllFactories()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_factories.clear();
}

bool ResourceManager::load(const ResourceBundle* bundle)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	const std::vector< std::pair< const TypeInfo*, Guid > >& resources = bundle->get();
	for (std::vector< std::pair< const TypeInfo*, Guid > >::const_iterator i = resources.begin(); i != resources.end(); ++i)
	{
		T_DEBUG(L"Preloading " << int32_t(1 + std::distance(resources.begin(), i)) << L" / " << int32_t(resources.size()) << L" " << i->first->getName());

		const IResourceFactory* factory = findFactoryFromResourceType(*i->first);
		if (!factory)
			return 0;

		bool cacheable = factory->isCacheable();
		if (!cacheable)
		{
			log::warning << L"Non cacheable resource cannot be preloaded; skipped" << Endl;
			continue;
		}

		Ref< ResidentResourceHandle >& residentHandle = m_residentHandles[i->second];
		if (residentHandle == 0 || residentHandle->get() == 0)
		{
			if (!residentHandle)
				residentHandle = new ResidentResourceHandle(*i->first, bundle->persistent());

			load(i->second, factory, *i->first, residentHandle);
			if (!residentHandle->get())
			{
				log::error << L"Unable to preload resource; skipped" << Endl;
				continue;
			}
		}
	}

	return true;
}

Ref< IResourceHandle > ResourceManager::bind(const TypeInfo& type, const Guid& guid)
{
	Ref< IResourceHandle > handle;

	if (guid.isNull() || !guid.isValid())
		return 0;

	const IResourceFactory* factory = findFactoryFromProductType(type);
	if (!factory)
		return 0;

	bool cacheable = factory->isCacheable();
	if (cacheable)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		std::map< Guid, Ref< ResidentResourceHandle > >::iterator i = m_residentHandles.find(guid);
		if (i != m_residentHandles.end())
			handle = i->second;
		else
		{
			Ref< ResidentResourceHandle > residentHandle = new ResidentResourceHandle(type, false);
			m_residentHandles[guid] = residentHandle;
			handle = residentHandle;
		}
	}
	else
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		RefArray< ExclusiveResourceHandle >& handles = m_exclusiveHandles[guid];

		// First try to reuse handles which are no longer in use.
		for (RefArray< ExclusiveResourceHandle >::iterator i = handles.begin(); i != handles.end(); ++i)
		{
			if (!(*i)->get())
			{
				handle = *i;
				break;
			}
		}

		if (!handle)
		{
			Ref< ExclusiveResourceHandle > exclusiveHandle = new ExclusiveResourceHandle(type);
			handles.push_back(exclusiveHandle);
			handle = exclusiveHandle;
		}
	}
	
	T_ASSERT (handle);

	if (!handle->get())
		load(guid, factory, type, handle);

	return handle;
}

void ResourceManager::reload(const Guid& guid)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	std::map< Guid, RefArray< ExclusiveResourceHandle > >::iterator i0 = m_exclusiveHandles.find(guid);
	if (i0 != m_exclusiveHandles.end())
	{
		const RefArray< ExclusiveResourceHandle >& handles = i0->second;
		for (RefArray< ExclusiveResourceHandle >::const_iterator i = handles.begin(); i != handles.end(); ++i)
		{
			const TypeInfo& productType = (*i)->getProductType();
			const IResourceFactory* factory = findFactoryFromProductType(productType);
			if (factory)
				load(guid, factory, productType, *i);
		}
		return;
	}

	std::map< Guid, Ref< ResidentResourceHandle > >::iterator i1 = m_residentHandles.find(guid);
	if (i1 != m_residentHandles.end())
	{
		const TypeInfo& productType = i1->second->getProductType();
		const IResourceFactory* factory = findFactoryFromProductType(productType);
		if (factory)
			load(guid, factory, productType, i1->second);
		return;
	}
}

void ResourceManager::reload(const TypeInfo& type)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	for (std::map< Guid, RefArray< ExclusiveResourceHandle > >::iterator i = m_exclusiveHandles.begin(); i != m_exclusiveHandles.end(); ++i)
	{
		const RefArray< ExclusiveResourceHandle >& handles = i->second;
		for (RefArray< ExclusiveResourceHandle >::const_iterator j = handles.begin(); j != handles.end(); ++j)
		{
			const TypeInfo& productType = (*j)->getProductType();
			if (is_type_of(type, productType))
			{
				const IResourceFactory* factory = findFactoryFromProductType(productType);
				if (factory)
				{
					(*j)->flush();
					load(i->first, factory, productType, *j);
				}
			}
		}
	}

	for (std::map< Guid, Ref< ResidentResourceHandle > >::iterator i = m_residentHandles.begin(); i != m_residentHandles.end(); ++i)
	{
		const TypeInfo& productType = i->second->getProductType();
		if (is_type_of(type, productType))
		{
			const IResourceFactory* factory = findFactoryFromProductType(productType);
			if (factory)
			{
				i->second->flush();
				load(i->first, factory, productType, i->second);
			}
		}
	}
}

void ResourceManager::unloadUnusedResident()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (std::map< Guid, Ref< ResidentResourceHandle > >::iterator i = m_residentHandles.begin(); i != m_residentHandles.end(); ++i)
	{
		T_ASSERT (i->second);
		if (
			!i->second->isPersistent() &&
			i->second->getReferenceCount() <= 1 &&
			i->second->get() != 0
		)
		{
			if (m_verbose)
				log::info << L"Unload resource \"" << i->first.format() << L"\" (" << type_name(i->second->get()) << L")" << Endl;
			i->second->replace(0);
		}
	}
}

void ResourceManager::getStatistics(ResourceManagerStatistics& outStatistics) const
{
	if (!m_lock.wait(0))
		return;

	outStatistics.residentCount = 0;
	for (std::map< Guid, Ref< ResidentResourceHandle > >::const_iterator i = m_residentHandles.begin(); i != m_residentHandles.end(); ++i)
	{
		if (i->second->get() != 0)
			++outStatistics.residentCount;
	}

	outStatistics.exclusiveCount = 0;
	for (std::map< Guid, RefArray< ExclusiveResourceHandle > >::const_iterator i = m_exclusiveHandles.begin(); i != m_exclusiveHandles.end(); ++i)
	{
		for (RefArray< ExclusiveResourceHandle >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			if (*j)
				++outStatistics.exclusiveCount;
		}
	}

	m_lock.release();
}

const IResourceFactory* ResourceManager::findFactoryFromResourceType(const TypeInfo& type)
{
	for (RefArray< const IResourceFactory >::iterator i = m_factories.begin(); i != m_factories.end(); ++i)
	{
		TypeInfoSet typeSet = (*i)->getResourceTypes();
		if (std::find(typeSet.begin(), typeSet.end(), &type) != typeSet.end())
			return *i;
	}
	return 0;
}

const IResourceFactory* ResourceManager::findFactoryFromProductType(const TypeInfo& type)
{
	for (RefArray< const IResourceFactory >::iterator i = m_factories.begin(); i != m_factories.end(); ++i)
	{
		TypeInfoSet typeSet = (*i)->getProductTypes();
		if (std::find(typeSet.begin(), typeSet.end(), &type) != typeSet.end())
			return *i;
	}
	return 0;
}

void ResourceManager::load(const Guid& guid, const IResourceFactory* factory, const TypeInfo& resourceType, IResourceHandle* handle)
{
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();

	// Do not attempt to load resource if thread has been stopped; application probably terminating
	// so we try to leave early.
	if (currentThread->stopped())
	{
		if (m_verbose)
			log::info << L"Resource loader thread stopped; skipped loading resource" << Endl;
		return;
	}

	m_timeStack.push(0.0);

	Timer timer;
	timer.start();

	Ref< Object > object = factory->create(this, resourceType, guid);
	if (object)
	{
		if (m_verbose)
			log::info << L"Resource \"" << guid.format() << L"\" (" << type_name(object) << L") created" << Endl;

		handle->replace(object);

		// Yield current thread; we want other threads to get some periodic CPU time to
		// render loading screens etc. Use sleep as we want lower priority threads also
		// to be able to run.
		currentThread->sleep(0);
	}
	else if (m_verbose)
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
