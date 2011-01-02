#ifndef traktor_world_ExternalEntityDataCache_H
#define traktor_world_ExternalEntityDataCache_H

#include <map>
#include "Core/Guid.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class EntityData;

class T_DLLCLASS ExternalEntityDataCache : public Object
{
	T_RTTI_CLASS;

public:
	void put(const Guid& externalGuid, EntityData* resolvedEntityData);

	EntityData* get(const Guid& externalGuid) const;

private:
	std::map< Guid, Ref< EntityData > > m_entityData;
};

	}
}

#endif	// traktor_world_ExternalEntityDataCache_H
