/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Physics/Body.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_BULLET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

// Bullet forward declarations.
class btCollisionShape;
class btTypedConstraint;
class btDynamicsWorld;
class btRigidBody;

namespace traktor::physics
{

struct IWorldCallback;
class Mesh;

/*!
 * \ingroup Bullet
 */
class T_DLLCLASS BodyBullet : public Body
{
	T_RTTI_CLASS;

public:
	explicit BodyBullet(
		const wchar_t* const tag,
		IWorldCallback* callback,
		btDynamicsWorld* dynamicsWorld,
		float timeScale,
		btRigidBody* body,
		btCollisionShape* shape,
		const Vector4& centerOfGravity,
		uint32_t collisionGroup,
		uint32_t collisionMask,
		int32_t material,
		const resource::Proxy< Mesh >& mesh
	);

	virtual void destroy() override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Transform getTransform() const override final;

	virtual Transform getCenterTransform() const override final;

	virtual void setKinematic(bool kinematic) override final;

	virtual bool isStatic() const override final;

	virtual bool isKinematic() const override final;

	virtual void setActive(bool active) override final;

	virtual bool isActive() const override final;

	virtual void setEnable(bool enable) override final;

	virtual bool isEnable() const override final;

	virtual void reset() override final;

	virtual void setMass(float mass, const Vector4& inertiaTensor) override final;

	virtual float getInverseMass() const override final;

	virtual Matrix33 getInertiaTensorInverseWorld() const override final;

	virtual void addForceAt(const Vector4& at, const Vector4& force, bool localSpace) override final;

	virtual void addTorque(const Vector4& torque, bool localSpace) override final;

	virtual void addLinearImpulse(const Vector4& linearImpulse, bool localSpace) override final;

	virtual void addAngularImpulse(const Vector4& angularImpulse, bool localSpace) override final;

	virtual void addImpulse(const Vector4& at, const Vector4& impulse, bool localSpace) override final;

	virtual void setLinearVelocity(const Vector4& linearVelocity) override final;

	virtual Vector4 getLinearVelocity() const override final;

	virtual void setAngularVelocity(const Vector4& angularVelocity) override final;

	virtual Vector4 getAngularVelocity() const override final;

	virtual Vector4 getVelocityAt(const Vector4& at, bool localSpace) const override final;

	virtual void setState(const BodyState& state) override final;

	virtual BodyState getState() const override final;

	virtual void integrate(float deltaTime) override final;

	void addConstraint(btTypedConstraint* constraint);

	void removeConstraint(btTypedConstraint* constraint);

	Transform getBodyTransform() const;

	void getFrictionAndRestitution(int32_t index, float& outFriction, float& outRestitution) const;

	btDynamicsWorld* getBtDynamicsWorld() const { return m_dynamicsWorld; }

	btRigidBody* getBtRigidBody() const { return m_body; }

	btCollisionShape* getBtCollisionShape() const { return m_shape; }

	uint32_t getCollisionGroup() const { return m_collisionGroup; }

	uint32_t getCollisionMask() const { return m_collisionMask; }

	int32_t getMaterial() const { return m_material; }

	const AlignedVector< btTypedConstraint* >& getJoints() const { return m_constraints; }

private:
	IWorldCallback* m_callback;
	btDynamicsWorld* m_dynamicsWorld;
	float m_timeScale;
	btRigidBody* m_body;
	btCollisionShape* m_shape;
	Vector4 m_centerOfGravity;
	uint32_t m_collisionGroup;
	uint32_t m_collisionMask;
	int32_t m_material;
	resource::Proxy< Mesh > m_mesh;
	AlignedVector< btTypedConstraint* > m_constraints;
	bool m_enable;
};

}
