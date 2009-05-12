#ifndef traktor_world_SpatialEntityData_H
#define traktor_world_SpatialEntityData_H

#include "World/Entity/EntityData.h"
#include "Core/Math/Matrix44.h"

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
	
	virtual void setTransform(const Matrix44& transform);
	
	virtual const Matrix44& getTransform() const;

	virtual bool serialize(Serializer& s);
	
private:
	Matrix44 m_transform;
};
	
	}
}

#endif	// traktor_world_SpatialEntityData_H
