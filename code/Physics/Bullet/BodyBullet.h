#ifndef traktor_physics_BodyBullet_H
#define traktor_physics_BodyBullet_H

#include "Physics/Body.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_BULLET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

// Bullet forward declarations.
class btCollisionShape;
class btDynamicsWorld;
class btRigidBody;

namespace traktor
{
	namespace physics
	{

struct IWorldCallback;
class Joint;

/*!
 * \ingroup Bullet
 */
class T_DLLCLASS BodyBullet : public Body
{
	T_RTTI_CLASS;

public:
	BodyBullet(
		IWorldCallback* callback,
		btDynamicsWorld* dynamicsWorld,
		btRigidBody* body,
		btCollisionShape* shape,
		const Vector4& centerOfGravity,
		uint32_t collisionGroup,
		uint32_t collisionMask,
		int32_t material
	);

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

	void addJoint(Joint* joint);

	void removeJoint(Joint* joint);

	Transform getBodyTransform() const;

	btDynamicsWorld* getBtDynamicsWorld() const { return m_dynamicsWorld; }

	btRigidBody* getBtRigidBody() const { return m_body; }

	btCollisionShape* getBtCollisionShape() const { return m_shape; }

	uint32_t getCollisionGroup() const { return m_collisionGroup; }

	uint32_t getCollisionMask() const { return m_collisionMask; }

	int32_t getMaterial() const { return m_material; }

	const std::vector< Joint* >& getJoints() const { return m_joints; }

private:
	IWorldCallback* m_callback;
	btDynamicsWorld* m_dynamicsWorld;
	btRigidBody* m_body;
	btCollisionShape* m_shape;
	Vector4 m_centerOfGravity;
	uint32_t m_collisionGroup;
	uint32_t m_collisionMask;
	int32_t m_material;
	std::vector< Joint* > m_joints;
	bool m_enable;
};

	}
}

#endif	// traktor_physics_BodyBullet_H
