#ifndef traktor_physics_BodyPhysX_H
#define traktor_physics_BodyPhysX_H

#include "Physics/Body.h"
#include "Physics/PhysX/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_PHYSX_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

struct DestroyCallbackPhysX;

/*!
 * \ingroup PhysX
 */
class T_DLLCLASS BodyPhysX : public Body
{
	T_RTTI_CLASS;

public:
	BodyPhysX(DestroyCallbackPhysX* callback, NxActor* actor);

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

	virtual void integrate();

	NxActor* getActor() const { return m_actor; }

private:
	DestroyCallbackPhysX* m_callback;
	NxActor* m_actor;
};

	}
}

#endif	// traktor_physics_BodyPhysX_H
