#include "Resource/ResourceCache.h"
#include "Resource/ResourceHandle.h"
#include "Core/Heap/New.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.resource.ResourceCache", ResourceCache, IResourceCache)

void ResourceCache::put(const Guid& guid, IResourceHandle* handle)
{
	m_cache[guid] = handle;
}

IResourceHandle* ResourceCache::get(const Guid& guid)
{
	return m_cache[guid];
}

void ResourceCache::flush(const Guid& guid)
{
	std::map< Guid, Ref< IResourceHandle > >::iterator i = m_cache.find(guid);
	if (i != m_cache.end())
	{
		Ref< IResourceHandle > handle = i->second;
		if (handle)
			handle->flush();
	}
}

void ResourceCache::flush()
{
	for (std::map< Guid, Ref< IResourceHandle > >::iterator i = m_cache.begin(); i != m_cache.end(); ++i)
	{
		Ref< IResourceHandle > handle = i->second;
		if (handle)
			handle->flush();
	}
}

	}
}
