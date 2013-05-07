#include "Amalgam/Classes/PhysicsClasses.h"
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
	namespace amalgam
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.CollisionContact", CollisionContact, Object)

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
			script::Any::fromObject(collisionInfo.body1),
			script::Any::fromObject(collisionInfo.body2),
			script::Any::fromObject(new CollisionContact(collisionInfo.contacts))
		};
		if (m_delegate)
			m_delegate->invoke(sizeof_array(argv), argv);
	}

private:
	Ref< script::Delegate > m_delegate;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.CollisionListener", CollisionListener, physics::CollisionListener)

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

	int32_t material() const
	{
		return m_result.material;
	}

private:
	physics::QueryResult m_result;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.QueryResult", QueryResult, Object)

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
	classQueryResult->addMethod("body", &QueryResult::body);
	classQueryResult->addMethod("position", &QueryResult::position);
	classQueryResult->addMethod("normal", &QueryResult::normal);
	classQueryResult->addMethod("distance", &QueryResult::distance);
	classQueryResult->addMethod("fraction", &QueryResult::fraction);
	classQueryResult->addMethod("material", &QueryResult::material);
	scriptManager->registerClass(classQueryResult);

	Ref< script::AutoScriptClass< physics::PhysicsManager > > classPhysicsManager = new script::AutoScriptClass< physics::PhysicsManager >();
	classPhysicsManager->addMethod("addCollisionListener", &physics::PhysicsManager::addCollisionListener);
	classPhysicsManager->addMethod("removeCollisionListener", &physics::PhysicsManager::removeCollisionListener);
	classPhysicsManager->addMethod("setGravity", &physics::PhysicsManager::setGravity);
	classPhysicsManager->addMethod("getGravity", &physics::PhysicsManager::getGravity);
	classPhysicsManager->addMethod("queryPoint", &physics_PhysicsManager_queryPoint);
	classPhysicsManager->addMethod("queryRay", &physics_PhysicsManager_queryRay);
	classPhysicsManager->addMethod("querySphere", &physics_PhysicsManager_querySphere);
	classPhysicsManager->addMethod("querySweep", &physics_PhysicsManager_querySweep_1);
	classPhysicsManager->addMethod("querySweep", &physics_PhysicsManager_querySweep_2);
	scriptManager->registerClass(classPhysicsManager);

	Ref< script::AutoScriptClass< physics::Body > > classBody = new script::AutoScriptClass< physics::Body >();
	classBody->addMethod("setTransform", &physics::Body::setTransform);
	classBody->addMethod("getTransform", &physics::Body::getTransform);
	classBody->addMethod("isStatic", &physics::Body::isStatic);
	classBody->addMethod("setActive", &physics::Body::setActive);
	classBody->addMethod("isActive", &physics::Body::isActive);
	classBody->addMethod("setEnable", &physics::Body::setEnable);
	classBody->addMethod("isEnable", &physics::Body::isEnable);
	classBody->addMethod("reset", &physics::Body::reset);
	classBody->addMethod("setMass", &physics::Body::setMass);
	classBody->addMethod("getInverseMass", &physics::Body::getInverseMass);
	classBody->addMethod("addForceAt", &physics::Body::addForceAt);
	classBody->addMethod("addTorque", &physics::Body::addTorque);
	classBody->addMethod("addLinearImpulse", &physics::Body::addLinearImpulse);
	classBody->addMethod("addAngularImpulse", &physics::Body::addAngularImpulse);
	classBody->addMethod("addImpulse", &physics::Body::addImpulse);
	classBody->addMethod("setLinearVelocity", &physics::Body::setLinearVelocity);
	classBody->addMethod("getLinearVelocity", &physics::Body::getLinearVelocity);
	classBody->addMethod("setAngularVelocity", &physics::Body::setAngularVelocity);
	classBody->addMethod("getAngularVelocity", &physics::Body::getAngularVelocity);
	classBody->addMethod("getVelocityAt", &physics::Body::getVelocityAt);
	classBody->addMethod("addCollisionListener", &physics::Body::addCollisionListener);
	classBody->addMethod("removeCollisionListener", &physics::Body::removeCollisionListener);
	classBody->addMethod("removeAllCollisionListeners", &physics::Body::removeAllCollisionListeners);
	classBody->addMethod("setUserObject", &physics::Body::setUserObject);
	classBody->addMethod("getUserObject", &physics::Body::getUserObject);
	scriptManager->registerClass(classBody);

	Ref< script::AutoScriptClass< physics::Joint > > classJoint = new script::AutoScriptClass< physics::Joint >();
	classJoint->addMethod("getBody1", &physics::Joint::getBody1);
	classJoint->addMethod("getBody2", &physics::Joint::getBody2);
	classJoint->addMethod("setEnable", &physics::Joint::setEnable);
	classJoint->addMethod("isEnable", &physics::Joint::isEnable);
	scriptManager->registerClass(classJoint);

	Ref< script::AutoScriptClass< physics::BallJoint > > classBallJoint = new script::AutoScriptClass< physics::BallJoint >();
	classBallJoint->addMethod("setAnchor", &physics::BallJoint::setAnchor);
	classBallJoint->addMethod("getAnchor", &physics::BallJoint::getAnchor);
	scriptManager->registerClass(classBallJoint);

	Ref< script::AutoScriptClass< physics::ConeTwistJoint > > classConeTwistJoint = new script::AutoScriptClass< physics::ConeTwistJoint >();
	scriptManager->registerClass(classConeTwistJoint);

	Ref< script::AutoScriptClass< physics::Hinge2Joint > > classHinge2Joint = new script::AutoScriptClass< physics::Hinge2Joint >();
	classHinge2Joint->addMethod("addTorques", &physics::Hinge2Joint::addTorques);
	classHinge2Joint->addMethod("getAngleAxis1", &physics::Hinge2Joint::getAngleAxis1);
	classHinge2Joint->addMethod("setVelocityAxis1", &physics::Hinge2Joint::setVelocityAxis1);
	classHinge2Joint->addMethod("setVelocityAxis2", &physics::Hinge2Joint::setVelocityAxis2);
	scriptManager->registerClass(classHinge2Joint);

	Ref< script::AutoScriptClass< physics::HingeJoint > > classHingeJoint = new script::AutoScriptClass< physics::HingeJoint >();
	classHingeJoint->addMethod("getAnchor", &physics::HingeJoint::getAnchor);
	classHingeJoint->addMethod("getAxis", &physics::HingeJoint::getAxis);
	classHingeJoint->addMethod("getAngle", &physics::HingeJoint::getAngle);
	classHingeJoint->addMethod("getAngleVelocity", &physics::HingeJoint::getAngleVelocity);
	scriptManager->registerClass(classHingeJoint);

	Ref< script::AutoScriptClass< physics::ArticulatedEntity > > classArticulatedEntity = new script::AutoScriptClass< physics::ArticulatedEntity >();
	classArticulatedEntity->addMethod("getEntities", &physics::ArticulatedEntity::getEntities);
	classArticulatedEntity->addMethod("getJoints", &physics::ArticulatedEntity::getJoints);
	scriptManager->registerClass(classArticulatedEntity);

	Ref< script::AutoScriptClass< physics::RigidEntity > > classRigidEntity = new script::AutoScriptClass< physics::RigidEntity >();
	classRigidEntity->addMethod("getBody", &physics::RigidEntity::getBody);
	classRigidEntity->addMethod("getEntity", &physics::RigidEntity::getEntity);
	scriptManager->registerClass(classRigidEntity);

	Ref< script::AutoScriptClass< CollisionContact > > classCollisionContact = new script::AutoScriptClass< CollisionContact >();
	classCollisionContact->addMethod("length", &CollisionContact::length);
	classCollisionContact->addMethod("position", &CollisionContact::position);
	classCollisionContact->addMethod("normal", &CollisionContact::normal);
	classCollisionContact->addMethod("depth", &CollisionContact::depth);
	scriptManager->registerClass(classCollisionContact);

	Ref< script::AutoScriptClass< physics::CollisionListener > > classCollisionListener = new script::AutoScriptClass< physics::CollisionListener >();
	scriptManager->registerClass(classCollisionListener);

	Ref< script::AutoScriptClass< CollisionListener > > classCollisionListenerDelegate = new script::AutoScriptClass< CollisionListener >();
	classCollisionListenerDelegate->addConstructor< script::Delegate* >();
	scriptManager->registerClass(classCollisionListenerDelegate);
}

	}
}
