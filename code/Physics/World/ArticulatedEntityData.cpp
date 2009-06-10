#include "Physics/World/ArticulatedEntityData.h"
#include "Physics/World/ArticulatedEntity.h"
#include "Physics/World/RigidEntity.h"
#include "Physics/PhysicsManager.h"
#include "Physics/JointDesc.h"
#include "World/Entity/EntityBuilder.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.physics.ArticulatedEntityData", ArticulatedEntityData, world::SpatialEntityData)

ArticulatedEntity* ArticulatedEntityData::createEntity(
	world::EntityBuilder* builder,
	PhysicsManager* physicsManager
) const
{
	RefArray< RigidEntity > entities;
	RefArray< Joint > joints;

	entities.resize(m_entities.size());
	for (uint32_t i = 0; i < uint32_t(m_entities.size()); ++i)
	{
		Ref< RigidEntity > entity = dynamic_type_cast< RigidEntity* >(builder->build(m_entities[i]));
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

	return gc_new< ArticulatedEntity >(
		cref(getTransform()),
		cref(entities),
		cref(joints)
	);
}

void ArticulatedEntityData::setTransform(const Matrix44& transform)
{
	Matrix44 deltaTransform = getTransform().inverseOrtho() * transform;
	for (RefArray< RigidEntityData >::iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		Matrix44 currentTransform = (*i)->getTransform();
		(*i)->setTransform(currentTransform * deltaTransform);
	}
	SpatialEntityData::setTransform(transform);
}

bool ArticulatedEntityData::serialize(Serializer& s)
{
	if (!world::SpatialEntityData::serialize(s))
		return false;

	s >> MemberRefArray< RigidEntityData >(L"entities", m_entities);
	s >> MemberStlVector< Constraint, MemberComposite< Constraint > >(L"constraints", m_constraints);

	return true;
}

ArticulatedEntityData::Constraint::Constraint()
:	entityIndex1(-1)
,	entityIndex2(-1)
{
}

bool ArticulatedEntityData::Constraint::serialize(Serializer& s)
{
	s >> MemberRef< JointDesc >(L"jointDesc", jointDesc);
	s >> Member< int32_t >(L"entityIndex1", entityIndex1);
	s >> Member< int32_t >(L"entityIndex2", entityIndex2);
	return true;
}

	}
}
