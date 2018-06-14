/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Database/Instance.h"
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

ResourceManager::ResourceManager(db::Database* database, bool verbose)
:	m_database(database)
,	m_verbose(verbose)
{
}

ResourceManager::~ResourceManager()
{
	destroy();
}

void ResourceManager::destroy()
{
	for (std::map< Guid, Ref< ResidentResourceHandle > >::iterator i = m_residentHandles.begin(); i != m_residentHandles.end(); ++i)
		i->second->replace(0);

	for (std::map< Guid, RefArray< ExclusiveResourceHandle > >::iterator i = m_exclusiveHandles.begin(); i != m_exclusiveHandles.end(); ++i)
	{
		for (RefArray< ExclusiveResourceHandle >::iterator j = i->second.begin(); j != i->second.end(); ++j)
			(*j)->replace(0);
		i->second.clear();
	}

	m_resourceFactories.clear();
	m_residentHandles.clear();
	m_exclusiveHandles.clear();
}

void ResourceManager::addFactory(const IResourceFactory* factory)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	{
		TypeInfoSet typeSet = factory->getResourceTypes();
		for (TypeInfoSet::const_iterator i = typeSet.begin(); i != typeSet.end(); ++i)
			m_resourceFactories.push_back(std::make_pair(*i, factory));
	}
}

void ResourceManager::removeFactory(const IResourceFactory* factory)
{
	T_FATAL_ERROR;
}

void ResourceManager::removeAllFactories()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_resourceFactories.clear();
}

bool ResourceManager::load(const ResourceBundle* bundle)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	const std::vector< std::pair< const TypeInfo*, Guid > >& resources = bundle->get();
	for (std::vector< std::pair< const TypeInfo*, Guid > >::const_iterator i = resources.begin(); i != resources.end(); ++i)
	{
		T_DEBUG(L"Preloading " << int32_t(1 + std::distance(resources.begin(), i)) << L" / " << int32_t(resources.size()) << L" " << i->first->getName());

		// Get resource instance from database.
		Ref< db::Instance > instance = m_database->getInstance(i->second);
		if (!instance)
		{
			log::error << L"Unable to preload resource " << i->second.format() << L"; no such instance." << Endl;
			return 0;
		}

		// Get type of resource.
		const TypeInfo* resourceType = instance->getPrimaryType();
		if (!resourceType)
		{
			log::error << L"Unable to preload resource " << i->second.format() << L"; unable to read resource type." << Endl;
			return 0;
		}

		// Find factory which can create products from resource.
		const IResourceFactory* factory = findFactory(*resourceType);
		if (!factory)
		{
			log::error << L"Unable to preload resource " << i->second.format() << L"; no factory for specified resource type \"" << resourceType->getName() << L"\"." << Endl;
			return 0;
		}

		// Determine product type; must be explicitly determined if we can safely preload the resource.
		TypeInfoSet productTypes = factory->getProductTypes(*resourceType);
		if (productTypes.size() != 1)
		{
			log::warning << L"Unable to preload resource " << i->second.format() << L"; unable to determine product type, skipped." << Endl;
			continue;
		}

		bool cacheable = factory->isCacheable(*i->first);
		if (!cacheable)
		{
			log::warning << L"Unable to preload resource " << i->second.format() << L"; resource non cacheable, skipped." << Endl;
			continue;
		}

		Ref< ResidentResourceHandle >& residentHandle = m_residentHandles[i->second];
		if (residentHandle == 0 || residentHandle->get() == 0)
		{
			const TypeInfo& productType = *(*productTypes.begin());

			if (!residentHandle)
				residentHandle = new ResidentResourceHandle(productType, bundle->persistent());

			load(instance, factory, productType, residentHandle);
			if (!residentHandle->get())
			{
				log::error << L"Unable to preload resource " << i->second.format() << L"; skipped." << Endl;
				continue;
			}
		}
	}

	return true;
}

Ref< ResourceHandle > ResourceManager::bind(const TypeInfo& productType, const Guid& guid)
{
	Ref< ResourceHandle > handle;

	if (guid.isNull() || !guid.isValid())
	{
		if (!guid.isNull())
			log::error << L"Unable to create " << productType.getName() << L" resource; invalid id." << Endl;
		return 0;
	}

	// Get resource instance from database.
	Ref< db::Instance > instance = m_database->getInstance(guid);
	if (!instance)
	{
		log::error << L"Unable to create " << productType.getName() << L" resource; no such instance (" << guid.format() << L")." << Endl;
		return 0;
	}

	// Get type of resource.
	const TypeInfo* resourceType = instance->getPrimaryType();
	if (!resourceType)
	{
		log::error << L"Unable to create " << productType.getName() << L" resource; unable to read resource type (" << guid.format() << L")." << Endl;
		return 0;
	}

	// Find factory which can create products from resource.
	const IResourceFactory* factory = findFactory(*resourceType);
	if (!factory)
	{
		log::error << L"Unable to create " << productType.getName() << L" resource; no factory for specified resource type \"" << resourceType->getName() << L"\" (" << guid.format() << L")." << Endl;
		return 0;
	}

	// Create resource handle.
	bool cacheable = factory->isCacheable(productType);
	if (cacheable)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		std::map< Guid, Ref< ResidentResourceHandle > >::iterator i = m_residentHandles.find(guid);
		if (i != m_residentHandles.end())
			handle = i->second;
		else
		{
			Ref< ResidentResourceHandle > residentHandle = new ResidentResourceHandle(productType, false);
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
			Ref< ExclusiveResourceHandle > exclusiveHandle = new ExclusiveResourceHandle(productType);
			handles.push_back(exclusiveHandle);
			handle = exclusiveHandle;
		}
	}
	T_ASSERT (handle);

	// If no resource loaded into handle then load resource through factory.
	if (!handle->get())
		load(instance, factory, productType, handle);

	return handle;
}

void ResourceManager::reload(const Guid& guid, bool flushedOnly)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (guid.isNull() || !guid.isValid())
		return;

	// Get resource instance from database.
	Ref< db::Instance > instance = m_database->getInstance(guid);
	if (!instance)
		return;

	// Get type of resource.
	const TypeInfo* resourceType = instance->getPrimaryType();
	if (!resourceType)
		return;

	// Find factory which can create products from resource.
	const IResourceFactory* factory = findFactory(*resourceType);
	if (!factory)
		return;

	std::map< Guid, Ref< ResidentResourceHandle > >::iterator i1 = m_residentHandles.find(guid);
	if (i1 != m_residentHandles.end())
	{
		const TypeInfo& productType = i1->second->getProductType();
		if (!flushedOnly || i1->second->get() == 0)
			load(instance, factory, productType, i1->second);
	}

	std::map< Guid, RefArray< ExclusiveResourceHandle > >::iterator i0 = m_exclusiveHandles.find(guid);
	if (i0 != m_exclusiveHandles.end())
	{
		const RefArray< ExclusiveResourceHandle >& handles = i0->second;
		for (RefArray< ExclusiveResourceHandle >::const_iterator i = handles.begin(); i != handles.end(); ++i)
		{
			const TypeInfo& productType = (*i)->getProductType();
			if (!flushedOnly || (*i)->get() == 0)
				load(instance, factory, productType, *i);
		}
	}
}

void ResourceManager::reload(const TypeInfo& productType, bool flushedOnly)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	for (std::map< Guid, RefArray< ExclusiveResourceHandle > >::iterator i = m_exclusiveHandles.begin(); i != m_exclusiveHandles.end(); ++i)
	{
		// Get resource instance from database.
		Ref< db::Instance > instance = m_database->getInstance(i->first);
		if (!instance)
			continue;

		// Get type of resource.
		const TypeInfo* resourceType = instance->getPrimaryType();
		if (!resourceType)
			continue;

		// Find factory which can create products from resource.
		const IResourceFactory* factory = findFactory(*resourceType);
		if (!factory)
			continue;

		// Reload all resources through factory.
		const RefArray< ExclusiveResourceHandle >& handles = i->second;
		for (RefArray< ExclusiveResourceHandle >::const_iterator j = handles.begin(); j != handles.end(); ++j)
		{
			const TypeInfo& handleProductType = (*j)->getProductType();
			if (is_type_of(productType, handleProductType))
			{
				if (!flushedOnly || (*j)->get() == 0)
				{
					(*j)->flush();
					load(instance, factory, handleProductType, *j);
				}
			}
		}
	}

	for (std::map< Guid, Ref< ResidentResourceHandle > >::iterator i = m_residentHandles.begin(); i != m_residentHandles.end(); ++i)
	{
		const TypeInfo& handleProductType = i->second->getProductType();
		if (is_type_of(productType, handleProductType))
		{
			// Get resource instance from database.
			Ref< db::Instance > instance = m_database->getInstance(i->first);
			if (!instance)
				continue;

			// Get type of resource.
			const TypeInfo* resourceType = instance->getPrimaryType();
			if (!resourceType)
				continue;

			// Find factory which can create products from resource.
			const IResourceFactory* factory = findFactory(*resourceType);
			if (!factory)
				continue;

			if (!flushedOnly || i->second->get() == 0)
			{
				i->second->flush();
				load(instance, factory, handleProductType, i->second);
			}
		}
	}
}

void ResourceManager::unload(const TypeInfo& productType)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	for (std::map< Guid, RefArray< ExclusiveResourceHandle > >::iterator i = m_exclusiveHandles.begin(); i != m_exclusiveHandles.end(); ++i)
	{
		const RefArray< ExclusiveResourceHandle >& handles = i->second;
		for (RefArray< ExclusiveResourceHandle >::const_iterator j = handles.begin(); j != handles.end(); ++j)
		{
			if (is_type_of(productType, (*j)->getProductType()))
				(*j)->flush();
		}
	}

	for (std::map< Guid, Ref< ResidentResourceHandle > >::iterator i = m_residentHandles.begin(); i != m_residentHandles.end(); ++i)
	{
		if (is_type_of(productType, i->second->getProductType()))
			i->second->flush();
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
				log::info << L"Unload resource \"" << i->first.format() << L"\" (" << type_name(i->second->get()) << L")." << Endl;
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

const IResourceFactory* ResourceManager::findFactory(const TypeInfo& resourceType) const
{
	for (std::vector< std::pair< const TypeInfo*, Ref< const IResourceFactory > > >::const_iterator i = m_resourceFactories.begin(); i != m_resourceFactories.end(); ++i)
	{
		if (is_type_of(*i->first, resourceType))
			return i->second;
	}
	return 0;
}

void ResourceManager::load(const db::Instance* instance, const IResourceFactory* factory, const TypeInfo& productType, ResourceHandle* handle)
{
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();

	// Do not attempt to load resource if thread has been stopped; application probably terminating
	// so we try to leave early.
	if (currentThread->stopped())
	{
		if (m_verbose)
			log::info << L"Resource loader thread stopped; skipped loading resource." << Endl;
		return;
	}

	Ref< Object > object = factory->create(this, m_database, instance, productType, handle->get());
	if (object)
	{
		if (m_verbose)
			log::info << L"Resource \"" << instance->getGuid().format() << L"\" (" << type_name(object) << L") created." << Endl;

		handle->replace(object);

		// Yield current thread; we want other threads to get some periodic CPU time to
		// render loading screens etc. Use sleep as we want lower priority threads also
		// to be able to run.
		currentThread->sleep(0);
	}
	else
		log::error << L"Unable to create resource \"" << instance->getGuid().format() << L"\" (" << productType.getName() << L") using factory \"" << type_name(factory) << L"\"." << Endl;
}

	}
}
