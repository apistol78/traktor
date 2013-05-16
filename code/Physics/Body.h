#ifndef traktor_physics_Body_H
#define traktor_physics_Body_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Matrix33.h"
#include "Core/Math/Transform.h"
#include "Physics/BodyState.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class CollisionListener;
struct CollisionInfo;

/*! \brief Rigid body.
 * \ingroup Physics
 */
class T_DLLCLASS Body : public Object
{
	T_RTTI_CLASS;

public:
	Body();

	/*! \brief Destroy rigid body. */
	virtual void destroy();

	/*! \brief Set rigid body world transform. */
	virtual void setTransform(const Transform& transform) = 0;

	/*! \brief Get rigid body world transform.
	 *
	 * \return World transform.
	 */
	virtual Transform getTransform() const = 0;

	/*! \brief Get rigid body world transform at center of gravity.
	 *
	 * \return World transform.
	 */
	virtual Transform getCenterTransform() const = 0;

	/*! \brief Get type of body. */
	virtual bool isStatic() const = 0;

	/*! \brief Get type of body. */
	virtual bool isKinematic() const = 0;

	/*! \brief Set rigid body active state.
	 *
	 * Activation state control if body is
	 * sleeping or is in active motion.
	 *
	 * \param active Active state.
	 */
	virtual void setActive(bool active) = 0;

	/*! \brief Get rigid body active state.
	 *
	 * \return Active state.
	 */
	virtual bool isActive() const = 0;

	/*! \brief Set rigid body enable state.
	 *
	 * Disabled bodies are removed from simulation
	 * thus is neither integrated nor colliding.
	 *
	 * \param enable Enable state.
	 */
	virtual void setEnable(bool enable) = 0;

	/*! \brief Get rigid body enable state.
	 *
	 * \return Enable state.
	 */
	virtual bool isEnable() const = 0;

	/*! \brief Reset body state. */
	virtual void reset() = 0;

	/*! \brief Set body mass. */
	virtual void setMass(float mass, const Vector4& inertiaTensor) = 0;

	/*! \brief Get inverse body mass. */
	virtual float getInverseMass() const = 0;

	/*! \brief Get inverse inertia tensor in world space. */
	virtual Matrix33 getInertiaTensorInverseWorld() const = 0;

	/*! \brief Apply force on body. */
	virtual void addForceAt(const Vector4& at, const Vector4& force, bool localSpace) = 0;

	/*! \brief Apply torque on body. */
	virtual void addTorque(const Vector4& torque, bool localSpace) = 0;

	/*! \brief Apply linear impulse on body. */
	virtual void addLinearImpulse(const Vector4& linearImpulse, bool localSpace) = 0;

	/*! \brief Apply angular impulse on body. */
	virtual void addAngularImpulse(const Vector4& angularImpulse, bool localSpace) = 0;

	/*! \brief Apply impulse on body. */
	virtual void addImpulse(const Vector4& at, const Vector4& impulse, bool localSpace) = 0;

	/*! \brief Set linear velocity of body. */
	virtual void setLinearVelocity(const Vector4& linearVelocity) = 0;

	/*! \brief Get linear velocity of body. */
	virtual Vector4 getLinearVelocity() const = 0;

	/*! \brief Set angular velocity of body. */
	virtual void setAngularVelocity(const Vector4& angularVelocity) = 0;

	/*! \brief Get angular velocity of body. */
	virtual Vector4 getAngularVelocity() const = 0;

	/*! \brief Get velocity at a given position of body. */
	virtual Vector4 getVelocityAt(const Vector4& at, bool localSpace) const = 0;

	/*! \brief Calculate constraint forces to match given body state. */
	virtual bool solveStateConstraint(const BodyState& state) = 0;

	/*! \brief Set body's complete state. */
	virtual bool setState(const BodyState& state) = 0;

	/*! \brief Get complete state of body. */
	virtual BodyState getState() const = 0;

	/*! \brief Add collision listener callback.
	 *
	 * Callbacks are issued every time a new collision
	 * pair is detected with this rigid body.
	 *
	 * \param collisionListener Collision listener.
	 */
	void addCollisionListener(CollisionListener* collisionListener);

	/*! \brief Remove collision listener callback.
	 *
	 * \param collisionListener Collision listener.
	 */
	void removeCollisionListener(CollisionListener* collisionListener);

	/*! \brief Remove all collision listener callbacks. */
	void removeAllCollisionListeners();

	/*! \brief Notify registered collision listeners.
	 *
	 * \param collisionInfo Information about new collision.
	 */
	void notifyCollisionListeners(const CollisionInfo& collisionInfo);

	/*! \brief Check if have any collision listeners. */
	bool haveCollisionListeners() const;

	/*! \brief Attach user defined object.
	 *
	 * \param userObject User object.
	 */
	void setUserObject(Object* userObject);

	/*! \brief Get attached user defined object.
	 *
	 * \return User object.
	 */
	Object* getUserObject() const;

	/*! \brief Set cluster id.
	 *
	 * Bodies within same cluster will never
	 * collide thus are useful for isolating
	 * articulated cluster of bodies.
	 */
	void setClusterId(uint32_t clusterId);

	/*! \brief Get cluster id.
	 *
	 * \return Cluster id.
	 */
	uint32_t getClusterId() const;

private:
	RefArray< CollisionListener > m_collisionListeners;
	Ref< Object > m_userObject;
	uint32_t m_clusterId;
};

	}
}

#endif	// traktor_physics_Body_H
