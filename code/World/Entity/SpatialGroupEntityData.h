#ifndef traktor_world_SpatialGroupEntityData_H
#define traktor_world_SpatialGroupEntityData_H

#include "Core/RefArray.h"
#include "World/Entity/SpatialEntityData.h"

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

class EntityInstance;

/*! \brief Spatial group entity data.
 * \ingroup World
 */
class T_DLLCLASS SpatialGroupEntityData : public SpatialEntityData
{
	T_RTTI_CLASS;

public:
	void addEntityData(SpatialEntityData* entityData);

	void removeEntityData(SpatialEntityData* entityData);

	void removeAllEntityData();

	RefArray< SpatialEntityData >& getEntityData();

	const RefArray< SpatialEntityData >& getEntityData() const;

	virtual void setTransform(const Transform& transform);
	
	virtual bool serialize(ISerializer& s);
	
private:
	RefArray< SpatialEntityData > m_entityData;
};
	
	}
}

#endif	// traktor_world_SpatialGroupEntityData_H
