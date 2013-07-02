#include <btBulletDynamicsCommon.h>
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Misc/InvokeOnce.h"
#include "Physics/BodyState.h"
#include "Physics/Joint.h"
#include "Physics/Bullet/Conversion.h"
#include "Physics/Bullet/BodyBullet.h"
#include "Physics/Bullet/Types.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

inline Vector4 convert(const BodyBullet* body, const Vector4& v, bool localSpace)
{
	T_ASSERT (!isNan((v).x()));
	T_ASSERT (!isNan((v).y()));
	T_ASSERT (!isNan((v).z()));
	T_ASSERT (!isNan((v).w()));
	return localSpace ? body->getTransform() * v : v;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.BodyBullet", BodyBullet, Body)

BodyBullet::BodyBullet(
	IWorldCallback* callback,
	btDynamicsWorld* dynamicsWorld,
	float simulationDeltaTime,
	btRigidBody* body,
	btCollisionShape* shape,
	const Vector4& centerOfGravity,
	uint32_t collisionGroup,
	uint32_t collisionMask,
	int32_t material
)
:	m_callback(callback)
,	m_dynamicsWorld(dynamicsWorld)
,	m_simulationDeltaTime(simulationDeltaTime)
,	m_body(body)
,	m_shape(shape)
,	m_centerOfGravity(centerOfGravity)
,	m_collisionGroup(collisionGroup)
,	m_collisionMask(collisionMask)
,	m_material(material)
,	m_enable(false)
{
}

void BodyBullet::destroy()
{
	invokeOnce< IWorldCallback, BodyBullet*, btRigidBody*, btCollisionShape* >(m_callback, &IWorldCallback::destroyBody, this, m_body, m_shape);

	m_dynamicsWorld = 0;
	m_body = 0;
	m_shape = 0;

	Body::destroy();
}

void BodyBullet::setTransform(const Transform& transform)
{
	btTransform bt = toBtTransform(transform * Transform(m_centerOfGravity));

	T_ASSERT (m_body);
	m_body->setWorldTransform(bt);

	// Update motion state's transform as well in case if kinematic body.
	if (m_body->isKinematicObject())
		m_body->getMotionState()->setWorldTransform(bt);
}

Transform BodyBullet::getTransform() const
{
	T_ASSERT (m_body);
	return fromBtTransform(m_body->getWorldTransform()) * Transform(-m_centerOfGravity);
}

Transform BodyBullet::getCenterTransform() const
{
	T_ASSERT (m_body);
	return fromBtTransform(m_body->getWorldTransform());
}

bool BodyBullet::isStatic() const
{
	return m_body->isStaticOrKinematicObject();
}

bool BodyBullet::isKinematic() const
{
	return m_body->isKinematicObject();
}

void BodyBullet::setActive(bool active)
{
	m_body->forceActivationState(active ? ACTIVE_TAG : ISLAND_SLEEPING);
}

bool BodyBullet::isActive() const
{
	return m_body->isActive();
}

void BodyBullet::setEnable(bool enable)
{
	if (enable == m_enable)
		return;

	if (!enable)
	{
		for (std::vector< btTypedConstraint* >::iterator i = m_constraints.begin(); i != m_constraints.end(); ++i)
			m_callback->removeConstraint(*i);
		m_callback->removeBody(m_body);
	}
	else
	{
		m_callback->insertBody(m_body, (uint16_t)m_collisionGroup, (uint16_t)m_collisionMask);
		for (std::vector< btTypedConstraint* >::iterator i = m_constraints.begin(); i != m_constraints.end(); ++i)
			m_callback->insertConstraint(*i);
	}

	m_enable = enable;
}

bool BodyBullet::isEnable() const
{
	return m_enable;
}

void BodyBullet::reset()
{
	T_ASSERT (m_body);
	m_body->clearForces();
	m_body->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
	m_body->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
}

void BodyBullet::setMass(float mass, const Vector4& inertiaTensor)
{
	T_ASSERT (m_body);
	T_ASSERT (!isNan(mass));
	m_body->setMassProps(mass, toBtVector3(inertiaTensor));
}

float BodyBullet::getInverseMass() const
{
	T_ASSERT (m_body);
	return m_body->getInvMass();
}

Matrix33 BodyBullet::getInertiaTensorInverseWorld() const
{
	T_ASSERT (m_body);
	return fromBtMatrix(m_body->getInvInertiaTensorWorld());
}

void BodyBullet::addForceAt(const Vector4& at, const Vector4& force, bool localSpace)
{
	T_ASSERT (m_body);

	Vector4 at0 = (localSpace ? (at + m_centerOfGravity) : at).xyz1();

	Vector4 at_ = convert(this, at0, localSpace);
	Vector4 force_ = convert(this, force, localSpace);
	Vector4 relativeAt = at_ - fromBtVector3(m_body->getCenterOfMassPosition(), 1.0f);

	m_body->applyForce(
		toBtVector3(force_),
		toBtVector3(relativeAt)
	);
}

void BodyBullet::addTorque(const Vector4& torque, bool localSpace)
{
	T_ASSERT (m_body);
	Vector4 torque_ = convert(this, torque, localSpace);
	m_body->applyTorque(toBtVector3(torque_));
}

void BodyBullet::addLinearImpulse(const Vector4& linearImpulse, bool localSpace)
{
	T_ASSERT (m_body);
	Vector4 linearImpulse_ = convert(this, linearImpulse, localSpace);
	m_body->applyCentralImpulse(toBtVector3(linearImpulse_));
}

void BodyBullet::addAngularImpulse(const Vector4& angularImpulse, bool localSpace)
{
	T_ASSERT (m_body);
	Vector4 angularImpulse_ = convert(this, angularImpulse, localSpace);
	m_body->applyTorqueImpulse(toBtVector3(angularImpulse_));
}

void BodyBullet::addImpulse(const Vector4& at, const Vector4& impulse, bool localSpace)
{
	T_ASSERT (m_body);

	Vector4 at0 = (localSpace ? (at + m_centerOfGravity) : at).xyz1();

	Vector4 at_ = convert(this, at0, localSpace);
	Vector4 impulse_ = convert(this, impulse, localSpace);
	Vector4 relativeAt = at_ - fromBtVector3(m_body->getCenterOfMassPosition(), 1.0f);

	m_body->applyImpulse(
		toBtVector3(impulse_),
		toBtVector3(relativeAt)
	);
}

void BodyBullet::setLinearVelocity(const Vector4& linearVelocity)
{
	T_ASSERT (m_body);
	m_body->setLinearVelocity(toBtVector3(linearVelocity));
}

Vector4 BodyBullet::getLinearVelocity() const
{
	T_ASSERT (m_body);
	return fromBtVector3(m_body->getLinearVelocity(), 0.0f);
}

void BodyBullet::setAngularVelocity(const Vector4& angularVelocity)
{
	T_ASSERT (m_body);
	m_body->setAngularVelocity(toBtVector3(angularVelocity));
}

Vector4 BodyBullet::getAngularVelocity() const
{
	T_ASSERT (m_body);
	return fromBtVector3(m_body->getAngularVelocity(), 0.0f);
}

Vector4 BodyBullet::getVelocityAt(const Vector4& at, bool localSpace) const
{
	T_ASSERT (m_body);

	Transform Tb = fromBtTransform(m_body->getWorldTransform());

	btVector3 relPos;
	if (localSpace)
		relPos = toBtVector3(Tb * at - Tb.translation());
	else
		relPos = toBtVector3(at - Tb.translation());

	return fromBtVector3(
		m_body->getVelocityInLocalPoint(relPos),
		0.0f
	);
}

bool BodyBullet::solveStateConstraint(const BodyState& state)
{
	Transform transform = getTransform();
	
	Vector4 linearError = state.getTransform().translation() - transform.translation();
	Vector4 angularError = (state.getTransform().rotation() * transform.rotation().inverse()).toAxisAngle();

	if (linearError.length() < 10.0f && angularError.length() < PI)
	{
		const Scalar tau(0.75f);
		setLinearVelocity(Scalar(1.0f / m_simulationDeltaTime) * linearError * tau + state.getLinearVelocity());
		setAngularVelocity(Scalar(1.0f / m_simulationDeltaTime) * angularError * tau + state.getAngularVelocity());
	}
	else
		setState(state);

	return true;
}

bool BodyBullet::setState(const BodyState& state)
{
	setTransform(state.getTransform());
	setLinearVelocity(state.getLinearVelocity());
	setAngularVelocity(state.getAngularVelocity());
	return true;
}

BodyState BodyBullet::getState() const
{
	BodyState state;
	state.setTransform(getTransform());
	state.setLinearVelocity(getLinearVelocity());
	state.setAngularVelocity(getAngularVelocity());
	return state;
}

void BodyBullet::integrate(float deltaTime)
{
	btTransform predictedTransform;
	m_body->applyCentralForce(m_dynamicsWorld->getGravity());
	m_body->integrateVelocities(deltaTime);
	m_body->predictIntegratedTransform(deltaTime, predictedTransform);
	m_body->setCenterOfMassTransform(predictedTransform);
	m_body->clearForces();
}

void BodyBullet::addConstraint(btTypedConstraint* constraint)
{
	m_constraints.push_back(constraint);
	if (m_enable)
		m_callback->insertConstraint(constraint);
}

void BodyBullet::removeConstraint(btTypedConstraint* constraint)
{
	std::vector< btTypedConstraint* >::iterator i = std::find(m_constraints.begin(), m_constraints.end(), constraint);
	if (i != m_constraints.end())
	{
		if (m_enable)
			m_callback->removeConstraint(constraint);
		m_constraints.erase(i);
	}
}

Transform BodyBullet::getBodyTransform() const
{
	T_ASSERT (m_body);
	return fromBtTransform(m_body->getWorldTransform());
}

	}
}
