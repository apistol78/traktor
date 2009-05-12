#ifndef traktor_physics_ArticulatedEntity_H
#define traktor_physics_ArticulatedEntity_H

#include "Core/Heap/Ref.h"
#include "World/Entity/SpatialGroupEntity.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class RigidEntity;
class Joint;

/*! \brief Articulated entity.
 * \ingroup Physics
 */
class T_DLLCLASS ArticulatedEntity : public world::SpatialGroupEntity
{
	T_RTTI_CLASS(ArticulatedEntity)

public:
	ArticulatedEntity(
		const Matrix44& transform,
		const RefArray< RigidEntity >& entities,
		const RefArray< Joint >& joints
	);

	virtual void destroy();

	const RefArray< Joint > getJoints() const { return m_joints; }

private:
	RefArray< Joint > m_joints;
};

	}
}

#endif	// traktor_physics_ArticulatedEntity_H
