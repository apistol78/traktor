#include "Parade/Classes/PhysicsClasses.h"
#include "Physics/BallJoint.h"
#include "Physics/Body.h"
#include "Physics/CollisionListener.h"
#include "Physics/ConeTwistJoint.h"
#include "Physics/Hinge2Joint.h"
#include "Physics/HingeJoint.h"
#include "Physics/PhysicsManager.h"
#include "Physics/World/ArticulatedEntity.h"
#include "Physics/World/RigidEntity.h"
#include "Script/AutoScriptClass.h"
#include "Script/Boxes.h"
#include "Script/Delegate.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace parade
	{
		namespace
		{

class CollisionContact : public Object
{
	T_RTTI_CLASS;

public:
	CollisionContact(const AlignedVector< physics::CollisionContact >& contacts)
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
	const AlignedVector< physics::CollisionContact >& m_contacts;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.CollisionContact", CollisionContact, Object)

class CollisionListener : public physics::CollisionListener
{
	T_RTTI_CLASS;

public:
	CollisionListener(script::Delegate* delegate)
	:	m_delegate(delegate)
	{
	}

	virtual void notify(const physics::CollisionInfo& collisionInfo)
	{
		script::Any argv[] =
		{
			script::Any(collisionInfo.body1),
			script::Any(collisionInfo.body2),
			script::Any(new CollisionContact(collisionInfo.contacts))
		};
		if (m_delegate)
			m_delegate->invoke(sizeof_array(argv), argv);
	}

private:
	Ref< script::Delegate > m_delegate;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.CollisionListener", CollisionListener, physics::CollisionListener)

class QueryResult : public Object
{
	T_RTTI_CLASS;

public:
	QueryResult(const physics::QueryResult& result)
	:	m_result(result)
	{
	}

	physics::Body* body() const
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

	uint32_t part() const
	{
		return m_result.part;
	}

private:
	physics::QueryResult m_result;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.QueryResult", QueryResult, Object)

Ref< QueryResult > physics_PhysicsManager_queryPoint(physics::PhysicsManager* this_, const Vector4& at, float margin)
{
	physics::QueryResult result;
	if (this_->queryPoint(at, margin, result))
		return new QueryResult(result);
	else
		return 0;
}

Ref< QueryResult > physics_PhysicsManager_queryRay(
	physics::PhysicsManager* this_,
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	uint32_t group,
	const physics::Body* ignoreBody,
	bool ignoreBackFace
)
{
	physics::QueryResult result;
	if (this_->queryRay(at, direction, maxLength, group, ignoreBody, ignoreBackFace, result))
		return new QueryResult(result);
	else
		return 0;
}

RefArray< physics::Body > physics_PhysicsManager_querySphere(
	physics::PhysicsManager* this_,
	const Vector4& at,
	float radius,
	uint32_t group,
	uint32_t queryTypes
)
{
	RefArray< physics::Body > bodies;
	this_->querySphere(at, radius, group, queryTypes, bodies);
	return bodies;
}

Ref< QueryResult > physics_PhysicsManager_querySweep_1(
	physics::PhysicsManager* this_,
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	float radius,
	uint32_t group,
	const physics::Body* ignoreBody
)
{
	physics::QueryResult result;
	if (this_->querySweep(at, direction, maxLength, radius, group, ignoreBody, result))
		return new QueryResult(result);
	else
		return 0;
}

Ref< QueryResult > physics_PhysicsManager_querySweep_2(
	physics::PhysicsManager* this_,
	const physics::Body* body,
	const Quaternion& orientation,
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	uint32_t group,
	const physics::Body* ignoreBody
)
{
	physics::QueryResult result;
	if (this_->querySweep(body, orientation, at, direction, maxLength, group, ignoreBody, result))
		return new QueryResult(result);
	else
		return 0;
}

		}

void registerPhysicsClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< QueryResult > > classQueryResult = new script::AutoScriptClass< QueryResult >();
	classQueryResult->addMethod(L"body", &QueryResult::body);
	classQueryResult->addMethod(L"position", &QueryResult::position);
	classQueryResult->addMethod(L"normal", &QueryResult::normal);
	classQueryResult->addMethod(L"distance", &QueryResult::distance);
	classQueryResult->addMethod(L"fraction", &QueryResult::fraction);
	classQueryResult->addMethod(L"part", &QueryResult::part);
	scriptManager->registerClass(classQueryResult);

	Ref< script::AutoScriptClass< physics::PhysicsManager > > classPhysicsManager = new script::AutoScriptClass< physics::PhysicsManager >();
	classPhysicsManager->addMethod(L"addCollisionListener", &physics::PhysicsManager::addCollisionListener);
	classPhysicsManager->addMethod(L"removeCollisionListener", &physics::PhysicsManager::removeCollisionListener);
	classPhysicsManager->addMethod(L"setGravity", &physics::PhysicsManager::setGravity);
	classPhysicsManager->addMethod(L"getGravity", &physics::PhysicsManager::getGravity);
	classPhysicsManager->addMethod(L"queryPoint", &physics_PhysicsManager_queryPoint);
	classPhysicsManager->addMethod(L"queryRay", &physics_PhysicsManager_queryRay);
	classPhysicsManager->addMethod(L"querySphere", &physics_PhysicsManager_querySphere);
	classPhysicsManager->addMethod(L"querySweep", &physics_PhysicsManager_querySweep_1);
	classPhysicsManager->addMethod(L"querySweep", &physics_PhysicsManager_querySweep_2);
	scriptManager->registerClass(classPhysicsManager);

	Ref< script::AutoScriptClass< physics::Body > > classBody = new script::AutoScriptClass< physics::Body >();
	classBody->addMethod(L"setTransform", &physics::Body::setTransform);
	classBody->addMethod(L"getTransform", &physics::Body::getTransform);
	classBody->addMethod(L"isStatic", &physics::Body::isStatic);
	classBody->addMethod(L"setActive", &physics::Body::setActive);
	classBody->addMethod(L"isActive", &physics::Body::isActive);
	classBody->addMethod(L"setEnable", &physics::Body::setEnable);
	classBody->addMethod(L"isEnable", &physics::Body::isEnable);
	classBody->addMethod(L"reset", &physics::Body::reset);
	classBody->addMethod(L"setMass", &physics::Body::setMass);
	classBody->addMethod(L"getInverseMass", &physics::Body::getInverseMass);
	classBody->addMethod(L"addForceAt", &physics::Body::addForceAt);
	classBody->addMethod(L"addTorque", &physics::Body::addTorque);
	classBody->addMethod(L"addLinearImpulse", &physics::Body::addLinearImpulse);
	classBody->addMethod(L"addAngularImpulse", &physics::Body::addAngularImpulse);
	classBody->addMethod(L"addImpulse", &physics::Body::addImpulse);
	classBody->addMethod(L"setLinearVelocity", &physics::Body::setLinearVelocity);
	classBody->addMethod(L"getLinearVelocity", &physics::Body::getLinearVelocity);
	classBody->addMethod(L"setAngularVelocity", &physics::Body::setAngularVelocity);
	classBody->addMethod(L"getAngularVelocity", &physics::Body::getAngularVelocity);
	classBody->addMethod(L"getVelocityAt", &physics::Body::getVelocityAt);
	classBody->addMethod(L"addCollisionListener", &physics::Body::addCollisionListener);
	classBody->addMethod(L"removeCollisionListener", &physics::Body::removeCollisionListener);
	classBody->addMethod(L"removeAllCollisionListeners", &physics::Body::removeAllCollisionListeners);
	classBody->addMethod(L"setUserObject", &physics::Body::setUserObject);
	classBody->addMethod(L"getUserObject", &physics::Body::getUserObject);
	scriptManager->registerClass(classBody);

	Ref< script::AutoScriptClass< physics::Joint > > classJoint = new script::AutoScriptClass< physics::Joint >();
	classJoint->addMethod(L"getBody1", &physics::Joint::getBody1);
	classJoint->addMethod(L"getBody2", &physics::Joint::getBody2);
	classJoint->addMethod(L"setEnable", &physics::Joint::setEnable);
	classJoint->addMethod(L"isEnable", &physics::Joint::isEnable);
	scriptManager->registerClass(classJoint);

	Ref< script::AutoScriptClass< physics::BallJoint > > classBallJoint = new script::AutoScriptClass< physics::BallJoint >();
	classBallJoint->addMethod(L"setAnchor", &physics::BallJoint::setAnchor);
	classBallJoint->addMethod(L"getAnchor", &physics::BallJoint::getAnchor);
	scriptManager->registerClass(classBallJoint);

	Ref< script::AutoScriptClass< physics::ConeTwistJoint > > classConeTwistJoint = new script::AutoScriptClass< physics::ConeTwistJoint >();
	scriptManager->registerClass(classConeTwistJoint);

	Ref< script::AutoScriptClass< physics::Hinge2Joint > > classHinge2Joint = new script::AutoScriptClass< physics::Hinge2Joint >();
	classHinge2Joint->addMethod(L"addTorques", &physics::Hinge2Joint::addTorques);
	classHinge2Joint->addMethod(L"getAngleAxis1", &physics::Hinge2Joint::getAngleAxis1);
	classHinge2Joint->addMethod(L"setVelocityAxis1", &physics::Hinge2Joint::setVelocityAxis1);
	classHinge2Joint->addMethod(L"setVelocityAxis2", &physics::Hinge2Joint::setVelocityAxis2);
	scriptManager->registerClass(classHinge2Joint);

	Ref< script::AutoScriptClass< physics::HingeJoint > > classHingeJoint = new script::AutoScriptClass< physics::HingeJoint >();
	classHingeJoint->addMethod(L"getAnchor", &physics::HingeJoint::getAnchor);
	classHingeJoint->addMethod(L"getAxis", &physics::HingeJoint::getAxis);
	classHingeJoint->addMethod(L"getAngle", &physics::HingeJoint::getAngle);
	classHingeJoint->addMethod(L"getAngleVelocity", &physics::HingeJoint::getAngleVelocity);
	scriptManager->registerClass(classHingeJoint);

	Ref< script::AutoScriptClass< physics::ArticulatedEntity > > classArticulatedEntity = new script::AutoScriptClass< physics::ArticulatedEntity >();
	classArticulatedEntity->addMethod(L"getEntities", &physics::ArticulatedEntity::getEntities);
	classArticulatedEntity->addMethod(L"getJoints", &physics::ArticulatedEntity::getJoints);
	scriptManager->registerClass(classArticulatedEntity);

	Ref< script::AutoScriptClass< physics::RigidEntity > > classRigidEntity = new script::AutoScriptClass< physics::RigidEntity >();
	classRigidEntity->addMethod(L"getBody", &physics::RigidEntity::getBody);
	classRigidEntity->addMethod(L"getEntity", &physics::RigidEntity::getEntity);
	scriptManager->registerClass(classRigidEntity);

	Ref< script::AutoScriptClass< CollisionContact > > classCollisionContact = new script::AutoScriptClass< CollisionContact >();
	classCollisionContact->addMethod(L"length", &CollisionContact::length);
	classCollisionContact->addMethod(L"position", &CollisionContact::position);
	classCollisionContact->addMethod(L"normal", &CollisionContact::normal);
	classCollisionContact->addMethod(L"depth", &CollisionContact::depth);
	scriptManager->registerClass(classCollisionContact);

	Ref< script::AutoScriptClass< physics::CollisionListener > > classCollisionListener = new script::AutoScriptClass< physics::CollisionListener >();
	scriptManager->registerClass(classCollisionListener);

	Ref< script::AutoScriptClass< CollisionListener > > classCollisionListenerDelegate = new script::AutoScriptClass< CollisionListener >();
	classCollisionListenerDelegate->addConstructor< script::Delegate* >();
	scriptManager->registerClass(classCollisionListenerDelegate);
}

	}
}
