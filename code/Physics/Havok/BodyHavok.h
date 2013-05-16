#ifndef traktor_physics_BodyHavok_H
#define traktor_physics_BodyHavok_H

#include "Physics/Body.h"
#include "Physics/Havok/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_HAVOK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

struct DestroyCallbackHavok;

/*!
 * \ingroup Havok
 */
class T_DLLCLASS BodyHavok : public Body
{
	T_RTTI_CLASS;

public:
	BodyHavok(DestroyCallbackHavok* callback, const HvkRef< hkpRigidBody >& rigidBody, float simulationDeltaTime);

	virtual void destroy();

	virtual void setTransform(const Transform& transform);

	virtual Transform getTransform() const;

	virtual Transform getCenterTransform() const;

	virtual bool isStatic() const;

	virtual bool isKinematic() const;

	virtual void setActive(bool active);

	virtual bool isActive() const;

	virtual void setEnable(bool enable);

	virtual bool isEnable() const;

	virtual void reset();

	virtual void setMass(float mass, const Vector4& inertiaTensor);

	virtual float getInverseMass() const;

	virtual Matrix33 getInertiaTensorInverseWorld() const;

	virtual void addForceAt(const Vector4& at, const Vector4& force, bool localSpace);

	virtual void addTorque(const Vector4& torque, bool localSpace);

	virtual void addLinearImpulse(const Vector4& linearImpulse, bool localSpace);

	virtual void addAngularImpulse(const Vector4& angularImpulse, bool localSpace);

	virtual void addImpulse(const Vector4& at, const Vector4& impulse, bool localSpace);

	virtual void setLinearVelocity(const Vector4& linearVelocity);

	virtual Vector4 getLinearVelocity() const;

	virtual void setAngularVelocity(const Vector4& angularVelocity);

	virtual Vector4 getAngularVelocity() const;

	virtual Vector4 getVelocityAt(const Vector4& at, bool localSpace) const;

	virtual bool solveStateConstraint(const BodyState& state);

	virtual bool setState(const BodyState& state);

	virtual BodyState getState() const;

	const HvkRef< hkpRigidBody >& getRigidBody() const { return m_rigidBody; }

private:
	DestroyCallbackHavok* m_callback;
	HvkRef< hkpRigidBody > m_rigidBody;
	float m_simulationDeltaTime;
};

	}
}

#endif	// traktor_physics_BodyHavok_H
