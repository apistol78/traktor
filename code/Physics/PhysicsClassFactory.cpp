/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Physics/BallJoint.h"
#include "Physics/Body.h"
#include "Physics/CollisionListener.h"
#include "Physics/ConeTwistJoint.h"
#include "Physics/Hinge2Joint.h"
#include "Physics/HingeJoint.h"
#include "Physics/PhysicsClassFactory.h"
#include "Physics/PhysicsManager.h"
#include "Physics/World/ArticulatedEntity.h"
#include "Physics/World/RigidEntity.h"
#include "Physics/World/Character/CharacterComponent.h"
#include "Physics/World/Vehicle/VehicleComponent.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

class CollisionContactWrapper : public Object
{
	T_RTTI_CLASS;

public:
	CollisionContactWrapper(const AlignedVector< CollisionContact >& contacts)
	:	m_contacts(contacts)
	{
	}

	int32_t length() const
	{
		return int32_t(m_contacts.size());
	}

	const Vector4& position(int32_t index) const
	{
		return m_contacts[index].position;
	}

	const Vector4& normal(int32_t index) const
	{
		return m_contacts[index].normal;
	}

	float depth(int32_t index) const
	{
		return m_contacts[index].depth;
	}

private:
	const AlignedVector< CollisionContact >& m_contacts;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.CollisionContact", CollisionContactWrapper, Object)

class CollisionListenerWrapper : public CollisionListener
{
	T_RTTI_CLASS;

public:
	CollisionListenerWrapper(IRuntimeDelegate* delegate)
	:	m_delegate(delegate)
	{
	}

	virtual void notify(const CollisionInfo& collisionInfo) T_OVERRIDE T_FINAL
	{
		Any argv[] =
		{
			Any::fromObject(collisionInfo.body1),
			Any::fromObject(collisionInfo.body2),
			Any::fromObject(new CollisionContactWrapper(collisionInfo.contacts))
		};
		if (m_delegate)
			m_delegate->call(sizeof_array(argv), argv);
	}

private:
	Ref< IRuntimeDelegate > m_delegate;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.CollisionListener", CollisionListenerWrapper, CollisionListener)

class QueryFilterWrapper : public Object
{
	T_RTTI_CLASS;

public:
	QueryFilterWrapper(uint32_t includeGroup)
	:	m_queryFilter(includeGroup)
	{
	}

	QueryFilterWrapper(uint32_t includeGroup, uint32_t ignoreGroup)
	:	m_queryFilter(includeGroup, ignoreGroup)
	{
	}

	QueryFilterWrapper(uint32_t includeGroup, uint32_t ignoreGroup, uint32_t ignoreClusterId)
	:	m_queryFilter(includeGroup, ignoreGroup, ignoreClusterId)
	{
	}

	operator const QueryFilter& () const
	{
		return m_queryFilter;
	}

private:
	QueryFilter m_queryFilter;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.QueryFilter", QueryFilterWrapper, Object)

class QueryResultWrapper : public Object
{
	T_RTTI_CLASS;

public:
	QueryResultWrapper(const QueryResult& result)
	:	m_result(result)
	{
	}

	Body* body() const
	{
		return m_result.body;
	}

	const Vector4& position() const
	{
		return m_result.position;
	}

	const Vector4& normal() const
	{
		return m_result.normal;
	}

	float distance() const
	{
		return m_result.distance;
	}

	float fraction() const
	{
		return m_result.fraction;
	}

	int32_t material() const
	{
		return m_result.material;
	}

private:
	QueryResult m_result;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.QueryResult", QueryResultWrapper, Object)

class BodyStateWrapper : public Object
{
	T_RTTI_CLASS;

public:
	BodyStateWrapper()
	{
	}

	BodyStateWrapper(const BodyState& state)
	:	m_state(state)
	{
	}

	operator const BodyState& () const
	{
		return m_state;
	}

private:
	BodyState m_state;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.BodyState", BodyStateWrapper, Object)

Ref< QueryResultWrapper > PhysicsManager_queryPoint(PhysicsManager* this_, const Vector4& at, float margin)
{
	QueryResult result;
	if (this_->queryPoint(at, margin, result))
		return new QueryResultWrapper(result);
	else
		return 0;
}

Ref< QueryResultWrapper > PhysicsManager_queryRay(
	PhysicsManager* this_,
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	const QueryFilterWrapper* queryFilter,
	bool ignoreBackFace
)
{
	QueryResult result;
	if (this_->queryRay(at, direction, maxLength, *queryFilter, ignoreBackFace, result))
		return new QueryResultWrapper(result);
	else
		return 0;
}

bool PhysicsManager_queryShadowRay(
	PhysicsManager* this_,
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	const QueryFilterWrapper* queryFilter,
	uint32_t queryTypes
)
{
	return this_->queryShadowRay(at, direction, maxLength, *queryFilter, queryTypes);
}

RefArray< Body > PhysicsManager_querySphere(
	PhysicsManager* this_,
	const Vector4& at,
	float radius,
	const QueryFilterWrapper* queryFilter,
	uint32_t queryTypes
)
{
	RefArray< Body > bodies;
	this_->querySphere(at, radius, *queryFilter, queryTypes, bodies);
	return bodies;
}

Ref< QueryResultWrapper > PhysicsManager_querySweep_1(
	PhysicsManager* this_,
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	float radius,
	const QueryFilterWrapper* queryFilter
)
{
	QueryResult result;
	if (this_->querySweep(at, direction, maxLength, radius, *queryFilter, result))
		return new QueryResultWrapper(result);
	else
		return 0;
}

Ref< QueryResultWrapper > PhysicsManager_querySweep_2(
	PhysicsManager* this_,
	const Body* body,
	const Quaternion& orientation,
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	const QueryFilterWrapper* queryFilter
)
{
	QueryResult result;
	if (this_->querySweep(body, orientation, at, direction, maxLength, *queryFilter, result))
		return new QueryResultWrapper(result);
	else
		return 0;
}

bool Body_setState(Body* this_, const BodyStateWrapper* state)
{
	if (state)
		return this_->setState(*state);
	else
		return false;
}

Ref< BodyStateWrapper > Body_getState(Body* this_)
{
	return new BodyStateWrapper(this_->getState());
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.PhysicsClassFactory", 0, PhysicsClassFactory, IRuntimeClassFactory)

void PhysicsClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< QueryFilterWrapper > > classQueryFilter = new AutoRuntimeClass< QueryFilterWrapper >();
	classQueryFilter->addConstructor< uint32_t >();
	classQueryFilter->addConstructor< uint32_t, uint32_t >();
	classQueryFilter->addConstructor< uint32_t, uint32_t, uint32_t >();
	registrar->registerClass(classQueryFilter);

	Ref< AutoRuntimeClass< QueryResultWrapper > > classQueryResult = new AutoRuntimeClass< QueryResultWrapper >();
	classQueryResult->addMethod("body", &QueryResultWrapper::body);
	classQueryResult->addMethod("position", &QueryResultWrapper::position);
	classQueryResult->addMethod("normal", &QueryResultWrapper::normal);
	classQueryResult->addMethod("distance", &QueryResultWrapper::distance);
	classQueryResult->addMethod("fraction", &QueryResultWrapper::fraction);
	classQueryResult->addMethod("material", &QueryResultWrapper::material);
	registrar->registerClass(classQueryResult);

	Ref< AutoRuntimeClass< BodyStateWrapper > > classBodyState = new AutoRuntimeClass< BodyStateWrapper >();
	registrar->registerClass(classBodyState);

	Ref< AutoRuntimeClass< PhysicsManager > > classPhysicsManager = new AutoRuntimeClass< PhysicsManager >();
	classPhysicsManager->addMethod("addCollisionListener", &PhysicsManager::addCollisionListener);
	classPhysicsManager->addMethod("removeCollisionListener", &PhysicsManager::removeCollisionListener);
	classPhysicsManager->addMethod("setGravity", &PhysicsManager::setGravity);
	classPhysicsManager->addMethod("getGravity", &PhysicsManager::getGravity);
	classPhysicsManager->addMethod("update", &PhysicsManager::update);
	classPhysicsManager->addMethod("getBodies", &PhysicsManager::getBodies);
	classPhysicsManager->addMethod("queryPoint", &PhysicsManager_queryPoint);
	classPhysicsManager->addMethod("queryRay", &PhysicsManager_queryRay);
	classPhysicsManager->addMethod("queryShadowRay", &PhysicsManager_queryShadowRay);
	classPhysicsManager->addMethod("querySphere", &PhysicsManager_querySphere);
	classPhysicsManager->addMethod("querySweep", &PhysicsManager_querySweep_1);
	classPhysicsManager->addMethod("querySweep", &PhysicsManager_querySweep_2);
	registrar->registerClass(classPhysicsManager);

	Ref< AutoRuntimeClass< Body > > classBody = new AutoRuntimeClass< Body >();
	classBody->addMethod("setTransform", &Body::setTransform);
	classBody->addMethod("getTransform", &Body::getTransform);
	classBody->addMethod("getCenterTransform", &Body::getCenterTransform);
	classBody->addMethod("isStatic", &Body::isStatic);
	classBody->addMethod("isKinematic", &Body::isKinematic);
	classBody->addMethod("setActive", &Body::setActive);
	classBody->addMethod("isActive", &Body::isActive);
	classBody->addMethod("setEnable", &Body::setEnable);
	classBody->addMethod("isEnable", &Body::isEnable);
	classBody->addMethod("reset", &Body::reset);
	classBody->addMethod("setMass", &Body::setMass);
	classBody->addMethod("getInverseMass", &Body::getInverseMass);
	classBody->addMethod("addForceAt", &Body::addForceAt);
	classBody->addMethod("addTorque", &Body::addTorque);
	classBody->addMethod("addLinearImpulse", &Body::addLinearImpulse);
	classBody->addMethod("addAngularImpulse", &Body::addAngularImpulse);
	classBody->addMethod("addImpulse", &Body::addImpulse);
	classBody->addMethod("setLinearVelocity", &Body::setLinearVelocity);
	classBody->addMethod("getLinearVelocity", &Body::getLinearVelocity);
	classBody->addMethod("setAngularVelocity", &Body::setAngularVelocity);
	classBody->addMethod("getAngularVelocity", &Body::getAngularVelocity);
	classBody->addMethod("getVelocityAt", &Body::getVelocityAt);
	classBody->addMethod("setState", &Body_setState);
	classBody->addMethod("getState", &Body_getState);
	classBody->addMethod("addCollisionListener", &Body::addCollisionListener);
	classBody->addMethod("removeCollisionListener", &Body::removeCollisionListener);
	classBody->addMethod("removeAllCollisionListeners", &Body::removeAllCollisionListeners);
	classBody->addMethod("setUserObject", &Body::setUserObject);
	classBody->addMethod("getUserObject", &Body::getUserObject);
	registrar->registerClass(classBody);

	Ref< AutoRuntimeClass< Joint > > classJoint = new AutoRuntimeClass< Joint >();
	classJoint->addMethod("getBody1", &Joint::getBody1);
	classJoint->addMethod("getBody2", &Joint::getBody2);
	registrar->registerClass(classJoint);

	Ref< AutoRuntimeClass< BallJoint > > classBallJoint = new AutoRuntimeClass< BallJoint >();
	classBallJoint->addMethod("setAnchor", &BallJoint::setAnchor);
	classBallJoint->addMethod("getAnchor", &BallJoint::getAnchor);
	registrar->registerClass(classBallJoint);

	Ref< AutoRuntimeClass< ConeTwistJoint > > classConeTwistJoint = new AutoRuntimeClass< ConeTwistJoint >();
	registrar->registerClass(classConeTwistJoint);

	Ref< AutoRuntimeClass< Hinge2Joint > > classHinge2Joint = new AutoRuntimeClass< Hinge2Joint >();
	classHinge2Joint->addMethod("addTorques", &Hinge2Joint::addTorques);
	classHinge2Joint->addMethod("getAngleAxis1", &Hinge2Joint::getAngleAxis1);
	classHinge2Joint->addMethod("setVelocityAxis1", &Hinge2Joint::setVelocityAxis1);
	classHinge2Joint->addMethod("setVelocityAxis2", &Hinge2Joint::setVelocityAxis2);
	registrar->registerClass(classHinge2Joint);

	Ref< AutoRuntimeClass< HingeJoint > > classHingeJoint = new AutoRuntimeClass< HingeJoint >();
	classHingeJoint->addMethod("getAnchor", &HingeJoint::getAnchor);
	classHingeJoint->addMethod("getAxis", &HingeJoint::getAxis);
	classHingeJoint->addMethod("getAngle", &HingeJoint::getAngle);
	classHingeJoint->addMethod("getAngleVelocity", &HingeJoint::getAngleVelocity);
	registrar->registerClass(classHingeJoint);

	Ref< AutoRuntimeClass< ArticulatedEntity > > classArticulatedEntity = new AutoRuntimeClass< ArticulatedEntity >();
	classArticulatedEntity->addMethod("getEntities", &ArticulatedEntity::getEntities);
	classArticulatedEntity->addMethod("getJoints", &ArticulatedEntity::getJoints);
	registrar->registerClass(classArticulatedEntity);

	Ref< AutoRuntimeClass< RigidEntity > > classRigidEntity = new AutoRuntimeClass< RigidEntity >();
	classRigidEntity->addMethod("getBody", &RigidEntity::getBody);
	classRigidEntity->addMethod("getEntity", &RigidEntity::getEntity);
	registrar->registerClass(classRigidEntity);

	Ref< AutoRuntimeClass< CharacterComponent > > classCharacterComponent = new AutoRuntimeClass< CharacterComponent >();
	registrar->registerClass(classCharacterComponent);

	Ref< AutoRuntimeClass< VehicleComponent > > classVehicleComponent = new AutoRuntimeClass< VehicleComponent >();
	classVehicleComponent->addMethod("setSteerAngle", &VehicleComponent::setSteerAngle);
	classVehicleComponent->addMethod("setEngineThrottle", &VehicleComponent::setEngineThrottle);
	registrar->registerClass(classVehicleComponent);

	Ref< AutoRuntimeClass< CollisionContactWrapper > > classCollisionContact = new AutoRuntimeClass< CollisionContactWrapper >();
	classCollisionContact->addMethod("length", &CollisionContactWrapper::length);
	classCollisionContact->addMethod("position", &CollisionContactWrapper::position);
	classCollisionContact->addMethod("normal", &CollisionContactWrapper::normal);
	classCollisionContact->addMethod("depth", &CollisionContactWrapper::depth);
	registrar->registerClass(classCollisionContact);

	Ref< AutoRuntimeClass< CollisionListenerWrapper > > classCollisionListener = new AutoRuntimeClass< CollisionListenerWrapper >();
	registrar->registerClass(classCollisionListener);

	Ref< AutoRuntimeClass< CollisionListenerWrapper > > classCollisionListenerDelegate = new AutoRuntimeClass< CollisionListenerWrapper >();
	classCollisionListenerDelegate->addConstructor< IRuntimeDelegate* >();
	registrar->registerClass(classCollisionListenerDelegate);
}

	}
}
