#include "Physics/World/ArticulatedEntityData.h"
#include "Physics/World/ArticulatedEntity.h"
#include "Physics/World/RigidEntity.h"
#include "Physics/PhysicsManager.h"
#include "Physics/JointDesc.h"
#include "World/Entity/IEntityBuilder.h"
#include "World/Entity/EntityInstance.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.ArticulatedEntityData", ArticulatedEntityData, world::SpatialEntityData)

Ref< ArticulatedEntity > ArticulatedEntityData::createEntity(
	world::IEntityBuilder* builder,
	PhysicsManager* physicsManager
) const
{
	RefArray< RigidEntity > entities;
	RefArray< Joint > joints;

	entities.resize(m_instances.size());
	for (uint32_t i = 0; i < uint32_t(m_instances.size()); ++i)
	{
		Ref< RigidEntity > entity = dynamic_type_cast< RigidEntity* >(builder->build(m_instances[i]));
		if (!entity)
			return 0;

		entities[i] = entity;
	}

	joints.resize(m_constraints.size());
	for (uint32_t i = 0; i < uint32_t(m_constraints.size()); ++i)
	{
		Ref< Body > body1, body2;

		if (m_constraints[i].entityIndex1 >= 0)
		{
			T_ASSERT_M (m_constraints[i].entityIndex1 < int32_t(entities.size()), L"Entity index 1 out of range");
			body1 = entities[m_constraints[i].entityIndex1]->getBody();
		}
		if (m_constraints[i].entityIndex2 >= 0)
		{
			T_ASSERT_M (m_constraints[i].entityIndex2 < int32_t(entities.size()), L"Entity index 2 out of range");
			body2 = entities[m_constraints[i].entityIndex2]->getBody();
		}

		if (body1)
		{
			Ref< Joint > joint = physicsManager->createJoint(
				m_constraints[i].jointDesc,
				getTransform(),
				body1,
				body2
			);
			if (!joint)
				return 0;

			joints[i] = joint;
		}
	}

	return new ArticulatedEntity(
		getTransform(),
		entities,
		joints
	);
}

void ArticulatedEntityData::setTransform(const Transform& transform)
{
	Transform deltaTransform = getTransform().inverse() * transform;
	for (RefArray< world::EntityInstance >::iterator i = m_instances.begin(); i != m_instances.end(); ++i)
	{
		world::SpatialEntityData* entityData = dynamic_type_cast< world::SpatialEntityData* >((*i)->getEntityData());
		if (entityData)
		{
			Transform currentTransform = entityData->getTransform();
			entityData->setTransform(currentTransform * deltaTransform);
		}
	}
	SpatialEntityData::setTransform(transform);
}

bool ArticulatedEntityData::serialize(ISerializer& s)
{
	if (!world::SpatialEntityData::serialize(s))
		return false;

	s >> MemberRefArray< world::EntityInstance >(L"instances", m_instances);
	s >> MemberStlVector< Constraint, MemberComposite< Constraint > >(L"constraints", m_constraints);

	return true;
}

ArticulatedEntityData::Constraint::Constraint()
:	entityIndex1(-1)
,	entityIndex2(-1)
{
}

bool ArticulatedEntityData::Constraint::serialize(ISerializer& s)
{
	s >> MemberRef< JointDesc >(L"jointDesc", jointDesc);
	s >> Member< int32_t >(L"entityIndex1", entityIndex1);
	s >> Member< int32_t >(L"entityIndex2", entityIndex2);
	return true;
}

	}
}
