#include <algorithm>
#include "Resource/ResourceManager.h"
#include "Resource/ResourceHandle.h"
#include "Resource/IResourceFactory.h"
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

	virtual Object* get() const { return 0; }

	virtual void flush() {}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.resource.ResourceManager", ResourceManager, IResourceManager)

ResourceManager::ResourceManager()
:	m_nullHandle(new NullResourceHandle())
{
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

	if (!cacheable)
		handle = new ResourceHandle(type);
	else
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		handle = m_cache[guid];
		if (!handle/* || &handle->getResourceType() != &type*/)
		{
			handle = new ResourceHandle(type);
			m_cache[guid] = handle;
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
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	std::map< Guid, Ref< ResourceHandle > >::iterator i = m_cache.find(guid);
	if (i == m_cache.end())
		return;

	Ref< ResourceHandle > handle = i->second;
	T_ASSERT (handle);

	handle->flush();
}

void ResourceManager::flush()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (std::map< Guid, Ref< ResourceHandle > >::iterator i = m_cache.begin(); i != m_cache.end(); ++i)
		i->second->flush();
}

void ResourceManager::dumpStatistics()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	log::debug << L"Resource manager statistics:" << Endl;
	log::debug << IncreaseIndent;
	
	log::debug << uint32_t(m_cache.size()) << L" cache entries" << Endl;
	
	uint32_t count = 0;
	for (std::map< Guid, Ref< ResourceHandle > >::iterator i = m_cache.begin(); i != m_cache.end(); ++i)
	{
		if (i->second->get())
			++count;
	}
	
	log::debug << count << L" resource(s)" << Endl;
	
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
	const TypeInfo& resourceType = handle->getResourceType();
	Ref< Object > object = factory->create(this, resourceType, guid);
	if (object)
	{
		T_ASSERT_M (is_type_of(resourceType, type_of(object)), L"Incorrect type of created resource");
		
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
