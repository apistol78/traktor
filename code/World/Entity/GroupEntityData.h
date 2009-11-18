#ifndef traktor_world_GroupEntityData_H
#define traktor_world_GroupEntityData_H

#include "Core/RefArray.h"
#include "World/Entity/EntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class EntityInstance;

/*! \brief Group entity data.
 * \ingroup World
 */
class T_DLLCLASS GroupEntityData : public EntityData
{
	T_RTTI_CLASS;

public:
	void addInstance(EntityInstance* instance);

	void removeInstance(EntityInstance* instance);

	void removeAllInstances();

	RefArray< EntityInstance >& getInstances();

	const RefArray< EntityInstance >& getInstances() const;
	
	virtual bool serialize(ISerializer& s);
	
private:
	RefArray< EntityInstance > m_instances;
};
	
	}
}

#endif	// traktor_world_GroupEntityData_H
