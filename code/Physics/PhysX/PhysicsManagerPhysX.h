#ifndef traktor_physics_PhysicsManagerPhysX_H
#define traktor_physics_PhysicsManagerPhysX_H

#include "Physics/PhysicsManager.h"
#include "Physics/PhysX/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_PHYSX_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

class NxPhysicsSDK;
class NxScene;

namespace traktor
{
	namespace physics
	{

class BodyPhysX;

/*!
 * \ingroup PhysX
 */
class T_DLLCLASS PhysicsManagerPhysX
:	public PhysicsManager
,	public DestroyCallbackPhysX
{
	T_RTTI_CLASS;

public:
	PhysicsManagerPhysX();

	virtual ~PhysicsManagerPhysX();

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
	NxPhysicsSDK* m_sdk;
	NxScene* m_scene;
	RefArray< BodyPhysX > m_bodies;

	virtual void destroyBody(Body* owner, NxActor& actor);

	virtual void destroyJoint(Joint* owner, NxJoint& joint);
};

	}
}

#endif	// traktor_physics_PhysicsManagerPhysX_H
