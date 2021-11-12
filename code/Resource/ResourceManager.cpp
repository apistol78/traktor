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
	for (auto& residentHandle : m_residentHandles)
		residentHandle.second->replace(nullptr);

	for (auto& exclusiveHandle : m_exclusiveHandles)
	{
		for (auto handle : exclusiveHandle.second)
			handle->replace(nullptr);
		exclusiveHandle.second.clear();
	}

	m_resourceFactories.clear();
	m_residentHandles.clear();
	m_exclusiveHandles.clear();
}

void ResourceManager::addFactory(const IResourceFactory* factory)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	{
		for (auto resourceType : factory->getResourceTypes())
			m_resourceFactories.push_back(std::make_pair(resourceType, factory));
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
	for (const auto& resource : bundle->get())
	{
		// Get resource instance from database.
		Ref< db::Instance > instance = m_database->getInstance(resource.second);
		if (!instance)
		{
			log::error << L"Unable to preload resource " << resource.second.format() << L"; no such instance." << Endl;
			return false;
		}

		// Get type of resource.
		const TypeInfo* resourceType = instance->getPrimaryType();
		if (!resourceType)
		{
			log::error << L"Unable to preload resource " << resource.second.format() << L"; unable to read resource type." << Endl;
			return false;
		}

		// Find factory which can create products from resource.
		const IResourceFactory* factory = findFactory(*resourceType);
		if (!factory)
		{
			log::error << L"Unable to preload resource " << resource.second.format() << L"; no factory for specified resource type \"" << resourceType->getName() << L"\"." << Endl;
			return false;
		}

		// Determine product type; must be explicitly determined if we can safely preload the resource.
		TypeInfoSet productTypes = factory->getProductTypes(*resourceType);
		if (productTypes.size() != 1)
		{
			log::warning << L"Unable to preload resource " << resource.second.format() << L"; unable to determine product type, skipped." << Endl;
			continue;
		}

		bool cacheable = factory->isCacheable(*resource.first);
		if (!cacheable)
		{
			log::warning << L"Unable to preload resource " << resource.second.format() << L"; resource non cacheable, skipped." << Endl;
			continue;
		}

		Ref< ResidentResourceHandle > residentHandle = m_residentHandles[resource.second];
		if (residentHandle == nullptr || residentHandle->get() == nullptr)
		{
			const TypeInfo& productType = *(*productTypes.begin());

			if (!residentHandle)
			{
				residentHandle = new ResidentResourceHandle(productType, bundle->persistent());
				m_residentHandles[resource.second] = residentHandle;
			}

			load(instance, factory, productType, residentHandle);

			if (!residentHandle->get())
			{
				log::error << L"Unable to preload resource " << resource.second.format() << L"; skipped." << Endl;
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
			log::error << L"Unable to bind a " << productType.getName() << L" resource; invalid id." << Endl;
		return nullptr;
	}

	// Get resource instance from database.
	Ref< db::Instance > instance = m_database->getInstance(guid);
	if (!instance)
	{
		log::error << L"Unable to bind a " << productType.getName() << L" resource; no such instance (" << guid.format() << L")." << Endl;
		return nullptr;
	}

	// Get type of resource.
	const TypeInfo* resourceType = instance->getPrimaryType();
	if (!resourceType)
	{
		log::error << L"Unable to bind a " << productType.getName() << L" resource; unable to read resource type (" << guid.format() << L")." << Endl;
		return nullptr;
	}

	// Find factory which can create products from resource.
	const IResourceFactory* factory = findFactory(*resourceType);
	if (!factory)
	{
		log::error << L"Unable to bind a " << productType.getName() << L" resource; no factory for instance type \"" << resourceType->getName() << L"\" (" << guid.format() << L")." << Endl;
		return nullptr;
	}

	// Create resource handle.
	bool cacheable = factory->isCacheable(productType);
	if (cacheable)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		auto it = m_residentHandles.find(guid);
		if (it != m_residentHandles.end())
			handle = it->second;
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
		for (auto h : handles)
		{
			if (!h->get())
			{
				handle = h;
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
	T_ASSERT(handle);

	// If no resource loaded into handle then load resource through factory.
	if (!handle->get())
		load(instance, factory, productType, handle);

	return handle;
}

bool ResourceManager::reload(const Guid& guid, bool flushedOnly)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (guid.isNull() || !guid.isValid())
		return false;

	// Get resource instance from database.
	Ref< db::Instance > instance = m_database->getInstance(guid);
	if (!instance)
		return false;

	// Get type of resource.
	const TypeInfo* resourceType = instance->getPrimaryType();
	if (!resourceType)
		return false;

	// Find factory which can create products from resource.
	const IResourceFactory* factory = findFactory(*resourceType);
	if (!factory)
		return false;

	bool loaded = false;

	auto i1 = m_residentHandles.find(guid);
	if (i1 != m_residentHandles.end())
	{
		const TypeInfo& productType = i1->second->getProductType();
		if (!flushedOnly || i1->second->get() == nullptr)
		{
			load(instance, factory, productType, i1->second);
			loaded = true;
		}
	}

	auto i0 = m_exclusiveHandles.find(guid);
	if (i0 != m_exclusiveHandles.end())
	{
		for (auto handle : i0->second)
		{
			const TypeInfo& productType = handle->getProductType();
			if (!flushedOnly || handle->get() == nullptr)
			{
				load(instance, factory, productType, handle);
				loaded = true;
			}
		}
	}

	return loaded;
}

void ResourceManager::reload(const TypeInfo& productType, bool flushedOnly)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	for (auto i = m_exclusiveHandles.begin(); i != m_exclusiveHandles.end(); ++i)
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
		for (auto handle : i->second)
		{
			const TypeInfo& handleProductType = handle->getProductType();
			if (is_type_of(productType, handleProductType))
			{
				if (!flushedOnly || handle->get() == nullptr)
				{
					handle->flush();
					load(instance, factory, handleProductType, handle);
				}
			}
		}
	}

	for (auto i = m_residentHandles.begin(); i != m_residentHandles.end(); ++i)
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

			if (!flushedOnly || i->second->get() == nullptr)
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
	for (auto& pair : m_exclusiveHandles)
	{
		for (auto handle : pair.second)
		{
			if (is_type_of(productType, handle->getProductType()))
				handle->flush();
		}
	}
	for (auto& pair : m_residentHandles)
	{
		if (is_type_of(productType, pair.second->getProductType()))
			pair.second->flush();
	}
}

void ResourceManager::unloadUnusedResident()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (auto& pair : m_residentHandles)
	{
		if (
			!pair.second->isPersistent() &&
			pair.second->getReferenceCount() <= 1 &&
			pair.second->get() != nullptr
		)
		{
			if (m_verbose)
				log::info << L"Unloading resource \"" << pair.first.format() << L"\" (" << type_name(pair.second->get()) << L")." << Endl;
			pair.second->replace(nullptr);
		}
	}
}

void ResourceManager::getStatistics(ResourceManagerStatistics& outStatistics) const
{
	if (!m_lock.wait(0))
		return;

	outStatistics.residentCount = 0;
	for (auto i = m_residentHandles.begin(); i != m_residentHandles.end(); ++i)
	{
		if (i->second->get() != nullptr)
			++outStatistics.residentCount;
	}

	outStatistics.exclusiveCount = 0;
	for (auto i = m_exclusiveHandles.begin(); i != m_exclusiveHandles.end(); ++i)
	{
		for (auto handle : i->second)
		{
			if (handle)
				++outStatistics.exclusiveCount;
		}
	}

	m_lock.release();
}

const IResourceFactory* ResourceManager::findFactory(const TypeInfo& resourceType) const
{
	for (auto i = m_resourceFactories.begin(); i != m_resourceFactories.end(); ++i)
	{
		if (is_type_of(*i->first, resourceType))
			return i->second;
	}
	return nullptr;
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
		// render loading screens etc.
		currentThread->yield();
	}
	else
		log::error << L"Unable to create resource \"" << instance->getGuid().format() << L"\" (" << productType.getName() << L") using factory \"" << type_name(factory) << L"\"." << Endl;
}

	}
}
