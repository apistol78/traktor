#include "Physics/Ode/DynamicBodyOde.h"
#include "Physics/Ode/Types.h"
#include "Physics/DynamicBodyState.h"
#include "Core/Math/Const.h"

// Access to ODE internals.
#include <objects.h>

namespace traktor
{
	namespace physics
	{
		namespace
		{

inline Vector4 convert(const DynamicBodyOde* body, const Vector4& v, bool localSpace)
{
	return localSpace ? body->getTransform() * v : v;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.DynamicBodyOde", DynamicBodyOde, DynamicBody)

DynamicBodyOde::DynamicBodyOde(
	DestroyCallback* callback,
	dWorldID worldId,
	dJointGroupID contactGroupId,
	dTriMeshDataID meshDataId,
	dGeomID geomId,
	dBodyID bodyId
)
:	m_callback(callback)
,	m_worldId(worldId)
,	m_contactGroupId(contactGroupId)
,	m_meshDataId(meshDataId)
,	m_geomId(geomId)
,	m_bodyId(bodyId)
{
	dGeomSetData(m_geomId, (void*)this);
}

DynamicBodyOde::~DynamicBodyOde()
{
	destroy();
}

void DynamicBodyOde::destroy()
{
	if (m_callback)
	{
		m_callback->bodyDestroyed(this);
		m_callback = 0;
	}
	if (m_bodyId)
	{
		dBodyDestroy(m_bodyId);
		m_bodyId = 0;
	}
	if (m_geomId)
	{
		dGeomDestroy(m_geomId);
		m_geomId = 0;
	}
}

void DynamicBodyOde::setTransform(const Matrix44& transform)
{
	T_ASSERT (m_bodyId != 0);

	const Vector4& p = transform.translation();
	dBodySetPosition(m_bodyId, p.x(), p.y(), p.z());

	dMatrix3 rotation =
	{
		transform(0, 0), transform(1, 0), transform(2, 0), 0.0f,
		transform(0, 1), transform(1, 1), transform(2, 1), 0.0f,
		transform(0, 2), transform(1, 2), transform(2, 2), 0.0f
	};
	dBodySetRotation(m_bodyId, rotation);
}

Matrix44 DynamicBodyOde::getTransform() const
{
	T_ASSERT (m_bodyId != 0);

	const dReal* p = dBodyGetPosition(m_bodyId);
	const dReal* r = dBodyGetRotation(m_bodyId);

	return Matrix44(
		r[0], r[4], r[8] , 0.0f,
		r[1], r[5], r[9] , 0.0f,
		r[2], r[6], r[10], 0.0f,
		p[0], p[1], p[2] , 1.0f
	);
}

void DynamicBodyOde::reset()
{
	T_ASSERT (m_bodyId != 0);

	dBodySetForce(m_bodyId, 0.0f, 0.0f, 0.0f);
	dBodySetTorque(m_bodyId, 0.0f, 0.0f, 0.0f);
	dBodySetLinearVel(m_bodyId, 0.0f, 0.0f, 0.0f);
	dBodySetAngularVel(m_bodyId, 0.0f, 0.0f, 0.0f);
}

void DynamicBodyOde::setMass(float mass, const Vector4& inertiaTensor)
{
}

float DynamicBodyOde::getInverseMass() const
{
	return m_bodyId->invMass;
}

Matrix33 DynamicBodyOde::getInertiaTensorInverseWorld() const
{
	const dReal* invI = m_bodyId->invI;
	Matrix33 inertiaTensorInverse(
		invI[0], invI[4], invI[8],
		invI[1], invI[5], invI[9],
		invI[2], invI[6], invI[10]
	);

	const dReal* r = dBodyGetRotation(m_bodyId);
	Matrix33 rotation(
		r[0], r[4], r[8],
		r[1], r[5], r[9],
		r[2], r[6], r[10]
	);

	return rotation.inverse() * inertiaTensorInverse * rotation;
}

void DynamicBodyOde::addForceAt(const Vector4& at, const Vector4& force, bool localSpace)
{
	T_ASSERT (m_bodyId != 0);

	Vector4 at_ = convert(this, at, localSpace);
	Vector4 force_ = convert(this, force, localSpace);

	dBodyAddForceAtPos(m_bodyId, force_.x(), force_.y(), force_.z(), at_.x(), at_.y(), at_.z());
}

void DynamicBodyOde::addTorque(const Vector4& torque, bool localSpace)
{
	T_ASSERT (m_bodyId != 0);

	Vector4 torque_ = convert(this, torque, localSpace);

	dBodyAddTorque(m_bodyId, torque_.x(), torque_.y(), torque_.z());
}

void DynamicBodyOde::addLinearImpulse(const Vector4& linearImpulse, bool localSpace)
{
	Vector4 linearImpulse_ = convert(this, linearImpulse, localSpace);
	T_ASSERT (abs(linearImpulse_.w()) < FUZZY_EPSILON);

	float invMass = getInverseMass();

	Vector4 linearVelocity = getLinearVelocity();
	linearVelocity += linearImpulse_ * Scalar(invMass);
	setLinearVelocity(linearVelocity);
}

void DynamicBodyOde::addAngularImpulse(const Vector4& angularImpulse, bool localSpace)
{
	Vector4 angularImpulse_ = convert(this, angularImpulse, localSpace);
	T_ASSERT (abs(angularImpulse_.w()) < FUZZY_EPSILON);

	Matrix33 inertiaTensorInvWorld_ = getInertiaTensorInverseWorld();
	Vector4 angularImpulse2_ = inertiaTensorInvWorld_ * angularImpulse_;

	Vector4 angularVelocity = getAngularVelocity();
	angularVelocity += angularImpulse2_;
	setAngularVelocity(angularVelocity);
}

void DynamicBodyOde::addImpulse(const Vector4& at, const Vector4& impulse, bool localSpace)
{
	Vector4 at_ = convert(this, at, localSpace);
	Vector4 impulse_ = convert(this, impulse, localSpace);

	// Apply linear impulse.
	addLinearImpulse(impulse_, false);

	// Apply angular impulse.
	Vector4 position = getTransform().translation();
	Vector4 relativePosition = at_ - position;
	if (relativePosition.length2() > FUZZY_EPSILON * FUZZY_EPSILON)
		addAngularImpulse(cross(relativePosition, impulse_), false);
}

void DynamicBodyOde::setLinearVelocity(const Vector4& linearVelocity)
{
	T_ASSERT (m_bodyId != 0);
	dBodySetLinearVel(m_bodyId, linearVelocity.x(), linearVelocity.y(), linearVelocity.z());
}

Vector4 DynamicBodyOde::getLinearVelocity() const
{
	T_ASSERT (m_bodyId != 0);

	const dReal* velocity = dBodyGetLinearVel(m_bodyId);
	return Vector4(velocity[0], velocity[1], velocity[2], 0.0f);
}

void DynamicBodyOde::setAngularVelocity(const Vector4& angularVelocity)
{
	T_ASSERT (m_bodyId != 0);
	dBodySetAngularVel(m_bodyId, angularVelocity.x(), angularVelocity.y(), angularVelocity.z());
}

Vector4 DynamicBodyOde::getAngularVelocity() const
{
	T_ASSERT (m_bodyId != 0);

	const dReal* velocity = dBodyGetAngularVel(m_bodyId);
	return Vector4(velocity[0], velocity[1], velocity[2], 0.0f);
}

Vector4 DynamicBodyOde::getVelocityAt(const Vector4& at, bool localSpace) const
{
	T_ASSERT (m_bodyId != 0);

	Vector4 at_ = convert(this, at, localSpace);
	T_ASSERT (abs(at_.w() - 1.0f) < FUZZY_EPSILON);

	dVector3 result;
	dBodyGetPointVel(m_bodyId, at_.x(), at_.y(), at_.z(), result);

	return Vector4(result[0], result[1], result[2], 0.0f);
}

bool DynamicBodyOde::setState(const DynamicBodyState& state)
{
	T_ASSERT (m_bodyId != 0);

	const Vector4& linearVel = state.getLinearVelocity();
	const Vector4& angularVel = state.getAngularVelocity();

	setTransform(state.getTransform());
	dBodySetLinearVel(m_bodyId, linearVel.x(), linearVel.y(), linearVel.z());
	dBodySetAngularVel(m_bodyId, angularVel.x(), angularVel.y(), angularVel.z());

	return true;
}

DynamicBodyState DynamicBodyOde::getState() const
{
	T_ASSERT (m_bodyId != 0);

	const dReal* linearVel = dBodyGetLinearVel(m_bodyId);
	const dReal* angularVel = dBodyGetAngularVel(m_bodyId);

	DynamicBodyState state;
	state.setTransform(getTransform());
	state.setLinearVelocity(Vector4(linearVel[0], linearVel[1], linearVel[2], 0.0f));
	state.setAngularVelocity(Vector4(angularVel[0], angularVel[1], angularVel[2], 0.0f));
	return state;
}

void DynamicBodyOde::setEnable(bool enable)
{
	T_ASSERT (m_bodyId != 0);

	if (enable)
		dBodyEnable(m_bodyId);
	else
		dBodyDisable(m_bodyId);
}

bool DynamicBodyOde::getEnable() const
{
	T_ASSERT (m_bodyId != 0);
	return bool(dBodyIsEnabled(m_bodyId) != 0);
}

const dGeomID DynamicBodyOde::getGeomId() const
{
	return m_geomId;
}

const dBodyID DynamicBodyOde::getBodyId() const
{
	return m_bodyId;
}

	}
}
