#include "World/Entity/ExternalEntityDataCache.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ExternalEntityDataCache", ExternalEntityDataCache, Object)

void ExternalEntityDataCache::put(const Guid& externalGuid, EntityData* resolvedEntityData)
{
	m_entityData.insert(std::make_pair(externalGuid, resolvedEntityData));
}

EntityData* ExternalEntityDataCache::get(const Guid& externalGuid) const
{
	std::map< Guid, Ref< EntityData > >::const_iterator i = m_entityData.find(externalGuid);
	return i != m_entityData.end() ? i->second : 0;
}

	}
}
