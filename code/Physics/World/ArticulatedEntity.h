/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_ArticulatedEntity_H
#define traktor_physics_ArticulatedEntity_H

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

/*! \brief Articulated entity.
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

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void update(const world::UpdateParams& update) T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual bool getTransform(Transform& outTransform) const T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	const RefArray< RigidEntity >& getEntities() const { return m_entities; }

	const RefArray< Joint >& getJoints() const { return m_joints; }

private:
	Transform m_transform;
	RefArray< RigidEntity > m_entities;
	RefArray< Joint > m_joints;
};

	}
}

#endif	// traktor_physics_ArticulatedEntity_H
