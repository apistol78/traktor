#ifndef traktor_physics_Body_H
#define traktor_physics_Body_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Math/Matrix44.h"

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
	T_RTTI_CLASS(Body)

public:
	/*! \brief Destroy rigid body. */
	virtual void destroy() = 0;

	/*! \brief Set rigid body world transform. */
	virtual void setTransform(const Matrix44& transform) = 0;

	/*! \brief Get rigid body world transform.
	 *
	 * \return World transform.
	 */
	virtual Matrix44 getTransform() const = 0;

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

	/*! \brief Notify registered collision listeners.
	 *
	 * \param collisionInfo Information about new collision.
	 */
	void notifyCollisionListeners(const CollisionInfo& collisionInfo);

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

private:
	RefArray< CollisionListener > m_collisionListeners;
	Ref< Object > m_userObject;
};

	}
}

#endif	// traktor_physics_Body_H
