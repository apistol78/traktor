#include <algorithm>
#include "Resource/ResourceCache.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.resource.ResourceCache", ResourceCache, IResourceCache)

void ResourceCache::put(const Guid& guid, Object* resource)
{
	m_cache[guid] = resource;
}

bool ResourceCache::get(const Guid& guid, Ref< Object >& outResource) const
{
	std::map< Guid, Ref< Object > >::const_iterator i = m_cache.find(guid);
	if (i == m_cache.end())
		return false;

	outResource = i->second;
	return true;
}

void ResourceCache::flush(const Guid& guid)
{
	for (std::map< Guid, Ref< Object > >::iterator i = m_cache.begin(); i != m_cache.end(); )
	{
		if (i->first == guid)
		{
			Heap::getInstance().invalidateRefs(i->second);
			T_ASSERT (!i->second);

			m_cache.erase(i++);
		}
		else
			++i;
	}
}

void ResourceCache::flush()
{
	for (std::map< Guid, Ref< Object > >::iterator i = m_cache.begin(); i != m_cache.end(); ++i)
	{
		if (i->second)
			Heap::getInstance().invalidateRefs(i->second);
		T_ASSERT (!i->second);
	}
	m_cache.clear();
}

	}
}
