#ifndef traktor_illuminate_IlluminateEntityData_H
#define traktor_illuminate_IlluminateEntityData_H

#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "World/EntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ILLUMINATE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace illuminate
	{

class T_DLLCLASS IlluminateEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	IlluminateEntityData();

	void addEntityData(world::EntityData* entityData);

	void removeEntityData(world::EntityData* entityData);

	void removeAllEntityData();

	virtual void serialize(ISerializer& s);
	
	virtual void setTransform(const Transform& transform);

	const Guid& getSeedGuid() const { return m_seedGuid; }

	const RefArray< world::EntityData >& getEntityData() const { return m_entityData; }

private:
	Guid m_seedGuid;
	RefArray< world::EntityData > m_entityData;
};

	}
}

#endif	// game_IlluminateEntityData_H
