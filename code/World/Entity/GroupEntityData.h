#ifndef traktor_world_GroupEntityData_H
#define traktor_world_GroupEntityData_H

#include "Core/RefArray.h"
#include "World/Entity/EntityData.h"

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

/*! \brief Group entity data.
 * \ingroup World
 */
class T_DLLCLASS GroupEntityData : public EntityData
{
	T_RTTI_CLASS;

public:
	void addEntityData(EntityData* entityData);

	void removeEntityData(EntityData* entityData);

	void removeAllEntityData();

	RefArray< EntityData >& getEntityData();

	const RefArray< EntityData >& getEntityData() const;
	
	virtual bool serialize(ISerializer& s);
	
private:
	RefArray< EntityData > m_entityData;
};
	
	}
}

#endif	// traktor_world_GroupEntityData_H
