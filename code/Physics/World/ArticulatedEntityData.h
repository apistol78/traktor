/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_ArticulatedEntityData_H
#define traktor_physics_ArticulatedEntityData_H

#include "Core/RefArray.h"
#include "World/EntityData.h"

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

	namespace physics
	{

class ArticulatedEntity;
class JointDesc;
class PhysicsManager;

/*! \brief Articulated entity data.
 * \ingroup Physics
 */
class T_DLLCLASS ArticulatedEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	struct Constraint
	{
		Ref< JointDesc > jointDesc;
		int32_t entityIndex1;
		int32_t entityIndex2;

		Constraint();

		void serialize(ISerializer& s);
	};

	Ref< ArticulatedEntity > createEntity(
		const world::IEntityBuilder* builder,
		PhysicsManager* physicsManager
	) const;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;
	
	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const RefArray< world::EntityData >& getEntityData() const { return m_entityData; }

	const std::vector< Constraint >& getConstraints() const { return m_constraints; }

private:
	RefArray< world::EntityData > m_entityData;
	std::vector< Constraint > m_constraints;
};

	}
}

#endif	// traktor_physics_ArticulatedEntityData_H
