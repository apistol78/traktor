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

class IEntityBuilder;
class EntityInstance;

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
	Ref< RigidEntity > createEntity(
		world::IEntityBuilder* builder,
		PhysicsManager* physicsManager
	) const;

	virtual bool serialize(Serializer& s);

	inline Ref< const BodyDesc > getBodyDesc() const { return m_bodyDesc; }

	inline Ref< const world::EntityInstance > getInstance() const { return m_instance; }

private:
	Ref< BodyDesc > m_bodyDesc;
	Ref< world::EntityInstance > m_instance;
};

	}
}

#endif	// traktor_physics_RigidEntityData_H
