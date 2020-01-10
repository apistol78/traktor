#pragma once

#include "Core/RefArray.h"
#include "World/Entity.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class RigidEntity;
class Joint;

/*! Articulated entity.
 * \ingroup Physics
 */
class T_DLLCLASS ArticulatedEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	ArticulatedEntity(
		const Transform& transform,
		const RefArray< RigidEntity >& entities,
		const RefArray< Joint >& joints
	);

	virtual void destroy() override final;

	virtual void update(const world::UpdateParams& update) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Transform getTransform() const override final;

	virtual Aabb3 getBoundingBox() const override final;

	const RefArray< RigidEntity >& getEntities() const { return m_entities; }

	const RefArray< Joint >& getJoints() const { return m_joints; }

private:
	Transform m_transform;
	RefArray< RigidEntity > m_entities;
	RefArray< Joint > m_joints;
};

	}
}

