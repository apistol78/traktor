#ifndef traktor_physics_DynamicBodyBullet_H
#define traktor_physics_DynamicBodyBullet_H

#include "Physics/DynamicBody.h"
#include "Physics/Bullet/BodyBullet.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_BULLET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

// Bullet forward declarations.
class btRigidBody;
class btCollisionShape;

namespace traktor
{
	namespace physics
	{

/*!
 * \ingroup Bullet
 */
class T_DLLCLASS DynamicBodyBullet : public BodyBullet< DynamicBody >
{
	T_RTTI_CLASS;

public:
	DynamicBodyBullet(
		IWorldCallback* callback,
		btDynamicsWorld* dynamicsWorld,
		btRigidBody* body,
		btCollisionShape* shape,
		uint32_t group
	);

	virtual void setTransform(const Transform& transform);

	virtual Transform getTransform() const;

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

	virtual bool setState(const DynamicBodyState& state);

	virtual DynamicBodyState getState() const;

	virtual void setActive(bool active);

	virtual bool isActive() const;
};

	}
}

#endif	// traktor_physics_DynamicBodyBullet_H
