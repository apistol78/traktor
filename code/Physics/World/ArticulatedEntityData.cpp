#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Physics/Joint.h"
#include "Physics/JointDesc.h"
#include "Physics/PhysicsManager.h"
#include "Physics/World/ArticulatedEntityData.h"
#include "Physics/World/ArticulatedEntity.h"
#include "Physics/World/RigidEntity.h"
#include "Physics/World/RigidEntityData.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.ArticulatedEntityData", 0, ArticulatedEntityData, world::EntityData)

Ref< ArticulatedEntity > ArticulatedEntityData::createEntity(
	const world::IEntityBuilder* builder,
	PhysicsManager* physicsManager
) const
{
	RefArray< RigidEntity > entities;
	RefArray< Joint > joints;

	entities.resize(m_entityData.size());
	for (uint32_t i = 0; i < uint32_t(m_entityData.size()); ++i)
	{
		Ref< RigidEntity > entity = dynamic_type_cast< RigidEntity* >(builder->create(m_entityData[i]));
		if (!entity)
			return 0;

		entities[i] = entity;
	}

	joints.resize(m_constraints.size());
	for (uint32_t i = 0; i < uint32_t(m_constraints.size()); ++i)
	{
		Ref< Body > body1, body2;

		if (m_constraints[i].entityIndex1 >= 0 && m_constraints[i].entityIndex1 < int32_t(entities.size()))
			body1 = entities[m_constraints[i].entityIndex1]->getBody();
		if (m_constraints[i].entityIndex2 >= 0 && m_constraints[i].entityIndex2 < int32_t(entities.size()))
			body2 = entities[m_constraints[i].entityIndex2]->getBody();

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
	Transform deltaTransform = transform * getTransform().inverse();
	for (RefArray< world::EntityData >::iterator i = m_entityData.begin(); i != m_entityData.end(); ++i)
	{
		Transform currentTransform = (*i)->getTransform();
		(*i)->setTransform(deltaTransform * currentTransform);
	}
	EntityData::setTransform(transform);
}

void ArticulatedEntityData::serialize(ISerializer& s)
{
	world::EntityData::serialize(s);

	s >> MemberRefArray< world::EntityData >(L"entityData", m_entityData);
	s >> MemberStlVector< Constraint, MemberComposite< Constraint > >(L"constraints", m_constraints);
}

ArticulatedEntityData::Constraint::Constraint()
:	entityIndex1(-1)
,	entityIndex2(-1)
{
}

void ArticulatedEntityData::Constraint::serialize(ISerializer& s)
{
	s >> MemberRef< JointDesc >(L"jointDesc", jointDesc);
	s >> Member< int32_t >(L"entityIndex1", entityIndex1);
	s >> Member< int32_t >(L"entityIndex2", entityIndex2);
}

	}
}
