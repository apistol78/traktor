#ifndef traktor_world_SpatialEntityData_H
#define traktor_world_SpatialEntityData_H

#include "World/Entity/EntityData.h"
#include "Core/Math/Transform.h"

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

/*! \brief Spatial entity data.
 * \ingroup World
 */
class T_DLLCLASS SpatialEntityData : public EntityData
{
	T_RTTI_CLASS(SpatialEntityData)
	
public:
	SpatialEntityData();
	
	virtual void setTransform(const Transform& transform);
	
	virtual const Transform& getTransform() const;

	virtual bool serialize(Serializer& s);
	
private:
	Transform m_transform;
};
	
	}
}

#endif	// traktor_world_SpatialEntityData_H
