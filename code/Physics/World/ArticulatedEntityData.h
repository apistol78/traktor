#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
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

/*! Articulated entity data.
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

	virtual void setTransform(const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const RefArray< world::EntityData >& getEntityData() const { return m_entityData; }

	const AlignedVector< Constraint >& getConstraints() const { return m_constraints; }

private:
	RefArray< world::EntityData > m_entityData;
	AlignedVector< Constraint > m_constraints;
};

	}
}

