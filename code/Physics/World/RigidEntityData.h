#ifndef traktor_physics_RigidEntityData_H
#define traktor_physics_RigidEntityData_H

#include "World/Entity/SpatialEntityData.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class IEntityBuilder;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace physics
	{

class BodyDesc;
class PhysicsManager;
class RigidEntity;

/*! \brief Rigid body entity data.
 * \ingroup Physics
 */
class T_DLLCLASS RigidEntityData : public world::SpatialEntityData
{
	T_RTTI_CLASS;

public:
	Ref< RigidEntity > createEntity(
		world::IEntityBuilder* builder,
		resource::IResourceManager* resourceManager,
		PhysicsManager* physicsManager
	) const;

	virtual bool serialize(ISerializer& s);

	inline Ref< const BodyDesc > getBodyDesc() const { return m_bodyDesc; }

	inline Ref< const world::SpatialEntityData > getEntityData() const { return m_entityData; }

private:
	Ref< BodyDesc > m_bodyDesc;
	Ref< world::SpatialEntityData > m_entityData;
};

	}
}

#endif	// traktor_physics_RigidEntityData_H
