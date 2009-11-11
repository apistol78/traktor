#include <algorithm>
#include "Resource/ResourceManager.h"
#include "Resource/ResourceHandle.h"
#include "Resource/IResourceFactory.h"
#include "Core/Heap/GcNew.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/Acquire.h"
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

	virtual Ref< Object > get() { return 0; }

	virtual void flush() {}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.resource.ResourceManager", ResourceManager, IResourceManager)

ResourceManager::ResourceManager()
:	m_nullHandle(gc_new< NullResourceHandle >())
{
}

void ResourceManager::addFactory(IResourceFactory* factory)
{
	Acquire< Semaphore > scope(m_lock);
	m_factories.push_back(factory);
}

void ResourceManager::removeFactory(IResourceFactory* factory)
{
	Acquire< Semaphore > scope(m_lock);
	m_factories.remove(factory);
}

void ResourceManager::removeAllFactories()
{
	Acquire< Semaphore > scope(m_lock);
	m_factories.clear();
}

Ref< IResourceHandle > ResourceManager::bind(const Type& type, const Guid& guid)
{
	Ref< ResourceHandle > handle;

	if (guid.isNull() || !guid.isValid())
		return m_nullHandle;

	Ref< IResourceFactory > factory = findFactory(type);
	if (!factory)
		return m_nullHandle;

	bool cacheable = factory->isCacheable();

	if (!cacheable)
		handle = gc_new< ResourceHandle >(cref(type));
	else
	{
		Acquire< Semaphore > scope(m_lock);
		handle = m_cache[guid];
		if (!handle)
		{
			handle = gc_new< ResourceHandle >(cref(type));
			m_cache[guid] = handle;
		}
	}
	
	T_ASSERT (handle);
	T_ASSERT_M(&handle->getResourceType() == &type, L"Incorrect resource type; already bound as another type");

	if (!handle->get())
		load(guid, factory, handle);

	return handle;
}

void ResourceManager::update(const Guid& guid, bool force)
{
	Acquire< Semaphore > scope(m_lock);

	std::map< Guid, Ref< ResourceHandle > >::iterator i = m_cache.find(guid);
	if (i == m_cache.end())
		return;

	Ref< ResourceHandle > handle = i->second;
	T_ASSERT (handle);

	if (!force && handle->get())
		return;

	Ref< IResourceFactory > factory = findFactory(handle->getResourceType());
	if (!factory)
		return;

	load(guid, factory, handle);
}

void ResourceManager::flush(const Guid& guid)
{
	Acquire< Semaphore > scope(m_lock);

	std::map< Guid, Ref< ResourceHandle > >::iterator i = m_cache.find(guid);
	if (i == m_cache.end())
		return;

	Ref< ResourceHandle > handle = i->second;
	T_ASSERT (handle);

	handle->flush();
}

void ResourceManager::flush()
{
	Acquire< Semaphore > scope(m_lock);
	for (std::map< Guid, Ref< ResourceHandle > >::iterator i = m_cache.begin(); i != m_cache.end(); ++i)
		i->second->flush();
}

Ref< IResourceFactory > ResourceManager::findFactory(const Type& type)
{
	for (RefArray< IResourceFactory >::iterator i = m_factories.begin(); i != m_factories.end(); ++i)
	{
		TypeSet typeSet = (*i)->getResourceTypes();
		if (std::find(typeSet.begin(), typeSet.end(), &type) != typeSet.end())
			return *i;
	}
	return 0;
}

void ResourceManager::load(const Guid& guid, IResourceFactory* factory, ResourceHandle* handle)
{
	const Type& resourceType = handle->getResourceType();
	Ref< Object > object = factory->create(this, resourceType, guid);
	if (object)
	{
		T_ASSERT_M (is_type_of(resourceType, object->getType()), L"Incorrect type of created resource");
		
		log::info << L"Resource \"" << guid.format() << L"\" (" << type_name(object) << L") created" << Endl;
		handle->replace(object);

		// Yield current thread; we want other threads to get some periodic CPU time to
		// render loading screens etc.
		ThreadManager::getInstance().getCurrentThread()->yield();
	}
	else
		log::error << L"Unable to create resource \"" << guid.format() << L"\" (" << resourceType.getName() << L")" << Endl;
}

	}
}
