#ifndef traktor_physics_RigidEntityData_H
#define traktor_physics_RigidEntityData_H

#include "Core/Heap/Ref.h"
#include "World/Entity/SpatialEntityData.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class EntityBuilder;

	}

	namespace physics
	{

class PhysicsManager;
class BodyDesc;
class RigidEntity;

/*! \brief Rigid body entity data.
 * \ingroup Physics
 */
class T_DLLCLASS RigidEntityData : public world::SpatialEntityData
{
	T_RTTI_CLASS(RigidEntityData)

public:
	RigidEntity* createEntity(
		world::EntityBuilder* builder,
		PhysicsManager* physicsManager
	) const;
	
	virtual void setTransform(const Matrix44& transform);

	virtual bool serialize(Serializer& s);

	inline const BodyDesc* getBodyDesc() const { return m_bodyDesc; }

	inline const world::SpatialEntityData* getEntityData() const { return m_entityData; }

private:
	Ref< BodyDesc > m_bodyDesc;
	Ref< world::SpatialEntityData > m_entityData;
};

	}
}

#endif	// traktor_physics_RigidEntityData_H
