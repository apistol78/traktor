#ifndef traktor_physics_PhysicsManagerBullet_H
#define traktor_physics_PhysicsManagerBullet_H

#include "Core/Thread/Semaphore.h"
#include "Physics/PhysicsManager.h"
#include "Physics/Bullet/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_BULLET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

// Bullet forward declarations.
class btCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btConstraintSolver;
class btDiscreteDynamicsWorld;
struct btBroadphasePair;
struct btDispatcherInfo;

namespace traktor
{
	namespace physics
	{

class BodyBullet;
class Joint;

/*!
 * \ingroup Bullet
 */
class T_DLLCLASS PhysicsManagerBullet
:	public PhysicsManager
,	public IWorldCallback
{
	T_RTTI_CLASS;

public:
	PhysicsManagerBullet();

	virtual ~PhysicsManagerBullet();

	virtual bool create(float simulationDeltaTime);

	virtual void destroy();

	virtual void setGravity(const Vector4& gravity);

	virtual Vector4 getGravity() const;

	virtual Ref< Body > createBody(resource::IResourceManager* resourceManager, const BodyDesc* desc);

	virtual Ref< Joint > createJoint(const JointDesc* desc, const Transform& transform, Body* body1, Body* body2);

	virtual void update(bool issueCollisionEvents);

	virtual void solveConstraints(const RefArray< Body >& bodies, const RefArray< Joint >& joints);

	virtual RefArray< Body > getBodies() const;

	virtual uint32_t getCollidingPairs(std::vector< CollisionPair >& outCollidingPairs) const;

	virtual bool queryPoint(
		const Vector4& at,
		float margin,
		QueryResult& outResult
	) const;

	virtual bool queryRay(
		const Vector4& at,
		const Vector4& direction,
		float maxLength,
		uint32_t group,
		const Body* ignoreBody,
		bool ignoreBackFace,
		QueryResult& outResult
	) const;

	virtual bool queryShadowRay(
		const Vector4& at,
		const Vector4& direction,
		float maxLength,
		uint32_t group,
		uint32_t queryTypes,
		const Body* ignoreBody
	) const;

	virtual uint32_t querySphere(
		const Vector4& at,
		float radius,
		uint32_t group,
		uint32_t queryTypes,
		RefArray< Body >& outBodies
	) const;

	virtual bool querySweep(
		const Vector4& at,
		const Vector4& direction,
		float maxLength,
		float radius,
		uint32_t group,
		const Body* ignoreBody,
		QueryResult& outResult
	) const;

	virtual bool querySweep(
		const Body* body,
		const Quaternion& orientation,
		const Vector4& at,
		const Vector4& direction,
		float maxLength,
		uint32_t group,
		const Body* ignoreBody,
		QueryResult& outResult
	) const;

	virtual void querySweep(
		const Vector4& at,
		const Vector4& direction,
		float maxLength,
		float radius,
		uint32_t group,
		const Body* ignoreBody,
		RefArray< Body >& outResult
	) const;

	virtual void queryOverlap(
		const Body* body,
		RefArray< Body >& outResult
	) const;

	virtual void getStatistics(PhysicsStatistics& outStatistics) const;

private:
	float m_simulationDeltaTime;
	btCollisionConfiguration* m_configuration;
	btCollisionDispatcher* m_dispatcher;
	btBroadphaseInterface* m_broadphase;
	btConstraintSolver* m_solver;
	btDiscreteDynamicsWorld* m_dynamicsWorld;
	mutable Semaphore m_lock;
	RefArray< BodyBullet > m_bodies;
	RefArray< Joint > m_joints;

	static PhysicsManagerBullet* ms_this;

	static void nearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo);

	virtual void insertBody(btRigidBody* rigidBody, uint16_t collisionGroup, uint16_t collisionFilter);

	virtual void removeBody(btRigidBody* rigidBody);

	virtual void insertConstraint(btTypedConstraint* constraint);

	virtual void removeConstraint(btTypedConstraint* constraint);

	virtual void destroyBody(BodyBullet* body, btRigidBody* rigidBody, btCollisionShape* shape);

	virtual void destroyConstraint(Joint* joint, btTypedConstraint* constraint);
};

	}
}

#endif	// traktor_physics_PhysicsManagerBullet_H
