#ifndef traktor_physics_ArticulatedEntityData_H
#define traktor_physics_ArticulatedEntityData_H

#include "Core/Heap/Ref.h"
#include "Physics/World/RigidEntityData.h"

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
class JointDesc;
class ArticulatedEntity;

/*! \brief Articulated entity data.
 * \ingroup Physics
 */
class T_DLLCLASS ArticulatedEntityData : public world::SpatialEntityData
{
	T_RTTI_CLASS(ArticulatedEntityData)

public:
	struct Constraint
	{
		Ref< JointDesc > jointDesc;
		int32_t entityIndex1;
		int32_t entityIndex2;

		Constraint();

		bool serialize(Serializer& s);
	};

	ArticulatedEntity* createEntity(
		world::IEntityBuilder* builder,
		PhysicsManager* physicsManager
	) const;

	virtual void setTransform(const Matrix44& transform);
	
	virtual bool serialize(Serializer& s);

	const RefArray< world::EntityInstance >& getInstances() const { return m_instances; }

	const std::vector< Constraint >& getConstraints() const { return m_constraints; }

private:
	RefArray< world::EntityInstance > m_instances;
	std::vector< Constraint > m_constraints;
};

	}
}

#endif	// traktor_physics_ArticulatedEntityData_H
