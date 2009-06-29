#include <algorithm>
#include "Resource/ResourceManager.h"
#include "Resource/ResourceCache.h"
#include "Resource/ResourceHandle.h"
#include "Resource/IResourceFactory.h"
#include "Core/Heap/New.h"
#include "Core/Thread/Acquire.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.resource.ResourceManager", ResourceManager, IResourceManager)

ResourceManager::ResourceManager()
:	m_cache(gc_new< ResourceCache >())
{
}

void ResourceManager::addFactory(IResourceFactory* factory)
{
	m_factories.push_back(factory);
}

void ResourceManager::removeFactory(IResourceFactory* factory)
{
	m_factories.remove(factory);
}

void ResourceManager::removeAllFactories()
{
	m_factories.clear();
}

void ResourceManager::setCache(IResourceCache* cache)
{
	Acquire< Semaphore > scope(m_lock);
	m_cache = cache;
}

IResourceCache* ResourceManager::getCache() const
{
	return m_cache;
}

IResourceHandle* ResourceManager::bind(const Type& type, const Guid& guid)
{
	Acquire< Semaphore > scope(m_lock);

	if (!m_cache)
		return 0;

	Ref< IResourceHandle > handle = m_cache->get(guid);

	if (!handle || !handle->get())
	{
		Ref< IResourceFactory > factory;
		bool cacheable = true;

		if (!handle)
			handle = gc_new< ResourceHandle >();

		for (RefList< IResourceFactory >::iterator i = m_factories.begin(); i != m_factories.end(); ++i)
		{
			TypeSet typeSet = (*i)->getResourceTypes();
			if (std::find(typeSet.begin(), typeSet.end(), &type) != typeSet.end())
			{
				factory = *i;
				break;
			}
		}

		if (factory)
		{
			Ref< Object > object = factory->create(this, type, guid, cacheable);
			if (object)
			{
				T_ASSERT_M (is_type_of(type, object->getType()), L"Incorrect type of created resource");

				handle->replace(object);

				if (cacheable)
				{
					log::info << L"Resource \"" << guid.format() << L"\" (" << type_name(object) << L") loaded" << Endl;
					m_cache->put(guid, handle);
				}
				else
					log::debug << L"Resource \"" << guid.format() << L"\" (" << type_name(object) << L") instantiated" << Endl;
			}
			else
				log::error << L"Unable to create resource \"" << guid.format() << L"\" (" << type.getName() << L")" << Endl;
		}
		else
			log::error << L"Unable to find resource factory for type \"" << type.getName() << L"\"" << Endl;
	}

	return handle;
}

	}
}
