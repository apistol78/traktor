#include "Resource/DualResourceCache.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.resource.DualResourceCache", DualResourceCache, IResourceCache)

void DualResourceCache::setPrimaryCache(IResourceCache* primaryCache)
{
	m_primaryCache = primaryCache;
}

IResourceCache* DualResourceCache::getPrimaryCache() const
{
	return m_primaryCache;
}

void DualResourceCache::setSecondaryCache(IResourceCache* secondaryCache)
{
	m_secondaryCache = secondaryCache;
}

IResourceCache* DualResourceCache::getSecondaryCache() const
{
	return m_secondaryCache;
}

void DualResourceCache::put(const Guid& guid, Object* resource)
{
	m_primaryCache->put(guid, resource);
}

bool DualResourceCache::get(const Guid& guid, Ref< Object >& outResource) const
{
	if (m_primaryCache->get(guid, outResource))
		return true;

	if (m_secondaryCache->get(guid, outResource))
		return true;

	return false;
}

void DualResourceCache::flush(const Guid& guid)
{
	m_primaryCache->flush(guid);
}

void DualResourceCache::flush()
{
	m_primaryCache->flush();
}

	}
}
