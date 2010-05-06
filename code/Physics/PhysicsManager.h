#ifndef traktor_physics_PhysicsManager_H
#define traktor_physics_PhysicsManager_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Transform.h"

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

struct CollisionInfo;
class CollisionListener;

class BodyDesc;
class Body;
class JointDesc;
class Joint;

/*! \brief Ray query result.
 * \ingroup Physics
 */
struct QueryResult
{
	Ref< Body > body;
	Vector4 position;
	Vector4 normal;
	float distance;
};

/*! \brief Collision pair.
 * \ingroup Physics
 */
struct CollisionPair
{
	Ref< Body > body1;
	Ref< Body > body2;
};

/*! \brief Physics manager.
 * \ingroup Physics
 */
class T_DLLCLASS PhysicsManager : public Object
{
	T_RTTI_CLASS;

public:
	enum QueryType
	{
		QtStatic = 1,
		QtDynamic = 2
	};

	/*! \brief Add collision listener.
	 *
	 * Collision listeners are invoked when
	 * there a near-collision between two rigid
	 * bodies are detected.
	 *
	 * \param collisionListener Collision listener instance.
	 */
	void addCollisionListener(CollisionListener* collisionListener);

	/*! \brief Remove collision listener.
	 *
	 * Remove previously registered collision listener.
	 *
	 * \param collisionListener Collision listener instance.
	 */
	void removeCollisionListener(CollisionListener* collisionListener);

	/*! \brief Notify collision listeners.
	 *
	 * Called by physics manager to notify
	 * registered collision listeners about
	 * detected near-collisions.
	 *
	 * \param collisionInfo Information about collision.
	 */
	void notifyCollisionListeners(const CollisionInfo& collisionInfo);

	/*! \brief Create physics manager.
	 *
	 * \param simulationDeltaTime Fixed simulation delta time.
	 */
	virtual bool create(float simulationDeltaTime) = 0;

	/*! \brief Destroy physics manager.
	 *
	 * Destroy physics manager including all
	 * rigid bodies.
	 */
	virtual void destroy() = 0;

	/*! \brief Set global gravity vector.
	 *
	 * \param gravity Gravity vector.
	 */
	virtual void setGravity(const Vector4& gravity) = 0;

	/*! \brief Get global gravity vector.
	 *
	 * \return Gravity vector.
	 */
	virtual Vector4 getGravity() const = 0;

	/*! \brief Create rigid body.
	 *
	 * \note Created body are initially disabled; need to call setEnable(true)
	 * in order to add to simulation.
	 *
	 * \param desc Rigid body description.
	 * \return Rigid body instance.
	 */
	virtual Ref< Body > createBody(const BodyDesc* desc) = 0;

	/*! \brief Create joint between bodies.
	 *
	 * Create a joint between bodies.
	 * If second body, body2, is null the joint
	 * is created with the world.
	 *
	 * \note Created joint are initially disabled; need to call setEnable(true)
	 * in order to add to simulation.
	 *
	 * \param desc Joint description.
	 * \param transform Joint transform.
	 * \param body1 First rigid body.
	 * \param body2 Second rigid body, can be null if joint is created with the world.
	 * \return Joint instance.
	 */
	virtual Ref< Joint > createJoint(const JointDesc* desc, const Transform& transform, Body* body1, Body* body2) = 0;

	/*! \brief Update simulation.
	 *
	 * Update entire simulation of the world,
	 * including physics, collision and joints.
	 * Time step is fixed and set when the physics
	 * manager is created.
	 */
	virtual void update() = 0;

	/*! \brief Get colliding pairs.
	 *
	 * Get all colliding pairs of bodies.
	 *
	 * \param outCollidingPairs Colliding pairs.
	 * \return Number of pairs.
	 */
	virtual uint32_t getCollidingPairs(std::vector< CollisionPair >& outCollidingPairs) const = 0;

	/*! \brief Get closest exit boundary.
	 *
	 * Check if point is within collision shape and
	 * return an approximate closest exit direction and distance.
	 *
	 * \param at Point position.
	 * \param margin Collision margin.
	 * \param outResult Query result.
	 * \return True if point is in collision shape and result was found.
	 */
	virtual bool queryPoint(const Vector4& at, float margin, QueryResult& outResult) const = 0;

	/*! \brief Ray cast world.
	 *
	 * Cast ray into the world and find closest
	 * intersection.
	 *
	 * \param at Ray origin in world space.
	 * \param direction Ray direction in world space.
	 * \param maxLength Maximum length of ray.
	 * \param ignoreBody Ignore ray casting body, can be null if no body is to be ignored.
	 * \param outResult Intersection result, only modified if method returns true.
	 * \return True if intersection found.
	 */
	virtual bool queryRay(const Vector4& at, const Vector4& direction, float maxLength, const Body* ignoreBody, QueryResult& outResult) const = 0;

	/*! \brief Get all bodies within a sphere.
	 *
	 * \param at Sphere origin in world space.
	 * \param radius Sphere radius.
	 * \param queryTypes Type of bodies, @sa QueryTypes
	 * \param outBodies Array of intersecting bodies.
	 * \return Number of bodies found.
	 */
	virtual uint32_t querySphere(const Vector4& at, float radius, uint32_t queryTypes, RefArray< Body >& outBodies) const = 0;

	/*! \brief Get closest contact from a swept sphere.
	 *
	 * \param at Sphere origin in world space.
	 * \param direction Sweep direction in world space.
	 * \param maxLength Max sweep length.
	 * \param radius Sphere radius.
	 * \param group Collision groups.
	 * \param ignoreBody Ignore ray casting body, can be null if no body is to be ignored.
	 * \param outResult Intersection result, only modified if method returns true.
	 * \return True if intersection found.
	 */
	virtual bool querySweep(
		const Vector4& at,
		const Vector4& direction,
		float maxLength,
		float radius,
		uint32_t group,
		const Body* ignoreBody,
		QueryResult& outResult
	) const = 0;

	/*! \brief Get number of bodies within world.
	 *
	 * This method is mostly used for debugging
	 * purposes to ensure bodies go to sleep etc.
	 *
	 * \param outCount Number of bodies.
	 * \param outActiveCount Number of active bodies.
	 */
	virtual void getBodyCount(uint32_t& outCount, uint32_t& outActiveCount) const = 0;

private:
	RefArray< CollisionListener > m_collisionListeners;
};

	}
}

#endif	// traktor_physics_PhysicsManager_H
