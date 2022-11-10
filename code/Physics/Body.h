/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Math/Matrix33.h"
#include "Core/Math/Transform.h"
#include "Physics/BodyState.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class CollisionListener;
struct CollisionInfo;

/*! Rigid body.
 * \ingroup Physics
 */
class T_DLLCLASS Body : public Object
{
	T_RTTI_CLASS;

public:
	/*! Destroy rigid body. */
	virtual void destroy();

	/*! Set rigid body world transform. */
	virtual void setTransform(const Transform& transform) = 0;

	/*! Get rigid body world transform.
	 *
	 * \return World transform.
	 */
	virtual Transform getTransform() const = 0;

	/*! Get rigid body world transform at center of gravity.
	 *
	 * \return World transform.
	 */
	virtual Transform getCenterTransform() const = 0;

	/*! Set body kinematic. */
	virtual void setKinematic(bool kinematic) = 0;

	/*! Get type of body. */
	virtual bool isStatic() const = 0;

	/*! Get type of body. */
	virtual bool isKinematic() const = 0;

	/*! Set rigid body active state.
	 *
	 * Activation state control if body is
	 * sleeping or is in active motion.
	 *
	 * \param active Active state.
	 */
	virtual void setActive(bool active) = 0;

	/*! Get rigid body active state.
	 *
	 * \return Active state.
	 */
	virtual bool isActive() const = 0;

	/*! Set rigid body enable state.
	 *
	 * Disabled bodies are removed from simulation
	 * thus is neither integrated nor colliding.
	 *
	 * \param enable Enable state.
	 */
	virtual void setEnable(bool enable) = 0;

	/*! Get rigid body enable state.
	 *
	 * \return Enable state.
	 */
	virtual bool isEnable() const = 0;

	/*! Reset body state. */
	virtual void reset() = 0;

	/*! Set body mass. */
	virtual void setMass(float mass, const Vector4& inertiaTensor) = 0;

	/*! Get inverse body mass. */
	virtual float getInverseMass() const = 0;

	/*! Get inverse inertia tensor in world space. */
	virtual Matrix33 getInertiaTensorInverseWorld() const = 0;

	/*! Apply force on body. */
	virtual void addForceAt(const Vector4& at, const Vector4& force, bool localSpace) = 0;

	/*! Apply torque on body. */
	virtual void addTorque(const Vector4& torque, bool localSpace) = 0;

	/*! Apply linear impulse on body. */
	virtual void addLinearImpulse(const Vector4& linearImpulse, bool localSpace) = 0;

	/*! Apply angular impulse on body. */
	virtual void addAngularImpulse(const Vector4& angularImpulse, bool localSpace) = 0;

	/*! Apply impulse on body. */
	virtual void addImpulse(const Vector4& at, const Vector4& impulse, bool localSpace) = 0;

	/*! Set linear velocity of body. */
	virtual void setLinearVelocity(const Vector4& linearVelocity) = 0;

	/*! Get linear velocity of body. */
	virtual Vector4 getLinearVelocity() const = 0;

	/*! Set angular velocity of body. */
	virtual void setAngularVelocity(const Vector4& angularVelocity) = 0;

	/*! Get angular velocity of body. */
	virtual Vector4 getAngularVelocity() const = 0;

	/*! Get velocity at a given position of body. */
	virtual Vector4 getVelocityAt(const Vector4& at, bool localSpace) const = 0;

	/*! Set body's complete state. */
	virtual bool setState(const BodyState& state) = 0;

	/*! Get complete state of body. */
	virtual BodyState getState() const = 0;

	/*! Perform manual integration of body. */
	virtual void integrate(float deltaTime) = 0;

	/*! Add collision listener callback.
	 *
	 * Callbacks are issued every time a new collision
	 * pair is detected with this rigid body.
	 *
	 * \param collisionListener Collision listener.
	 */
	void addCollisionListener(CollisionListener* collisionListener);

	/*! Remove collision listener callback.
	 *
	 * \param collisionListener Collision listener.
	 */
	void removeCollisionListener(CollisionListener* collisionListener);

	/*! Remove all collision listener callbacks. */
	void removeAllCollisionListeners();

	/*! Notify registered collision listeners.
	 *
	 * \param collisionInfo Information about new collision.
	 */
	void notifyCollisionListeners(const CollisionInfo& collisionInfo);

	/*! Check if have any collision listeners. */
	bool haveCollisionListeners() const { return !m_collisionListeners.empty(); }

	/*! Attach user defined object.
	 *
	 * \param userObject User object.
	 */
	void setUserObject(Object* userObject) { m_userObject = userObject; }

	/*! Get attached user defined object.
	 *
	 * \return User object.
	 */
	Object* getUserObject() const { return m_userObject; }

	/*! Set cluster id.
	 *
	 * Bodies within same cluster will never
	 * collide thus are useful for isolating
	 * articulated cluster of bodies.
	 */
	void setClusterId(uint32_t clusterId) { m_clusterId = clusterId; }

	/*! Get cluster id.
	 *
	 * \return Cluster id.
	 */
	uint32_t getClusterId() const { return m_clusterId; }

	/*! Get debug tag.
	 */
	const std::wstring& getTag() const { return m_tag; }

protected:
	Body(const wchar_t* const tag);

private:
	std::wstring m_tag;
	RefArray< CollisionListener > m_collisionListeners;
	Ref< Object > m_userObject;
	uint32_t m_clusterId;
};

	}
}

