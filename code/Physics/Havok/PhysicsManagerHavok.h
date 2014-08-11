#ifndef traktor_physics_PhysicsManagerHavok_H
#define traktor_physics_PhysicsManagerHavok_H

#include "Physics/PhysicsManager.h"
#include "Physics/Havok/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_HAVOK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

class hkPoolMemory;
class hkThreadMemory;
class hkpWorld;

namespace traktor
{
	namespace physics
	{

class BodyHavok;

/*!
 * \ingroup Havok
 */
class T_DLLCLASS PhysicsManagerHavok
:	public PhysicsManager
,	public DestroyCallbackHavok
{
	T_RTTI_CLASS;

public:
	PhysicsManagerHavok();

	virtual ~PhysicsManagerHavok();

	virtual bool create(float simulationDeltaTime, float timeScale);

	virtual void destroy();

	virtual void setGravity(const Vector4& gravity);

	virtual Vector4 getGravity() const;

	virtual Ref< Body > createBody(resource::IResourceManager* resourceManager, const BodyDesc* desc, const wchar_t* const tag);

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
		uint32_t ignoreClusterId,
		bool ignoreBackFace,
		QueryResult& outResult
	) const;

	virtual bool queryShadowRay(
		const Vector4& at,
		const Vector4& direction,
		float maxLength,
		uint32_t group,
		uint32_t queryTypes,
		uint32_t ignoreClusterId
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
		uint32_t ignoreClusterId,
		QueryResult& outResult
	) const;

	virtual bool querySweep(
		const Body* body,
		const Quaternion& orientation,
		const Vector4& at,
		const Vector4& direction,
		float maxLength,
		uint32_t group,
		uint32_t ignoreClusterId,
		QueryResult& outResult
	) const;

	virtual void querySweep(
		const Vector4& at,
		const Vector4& direction,
		float maxLength,
		float radius,
		uint32_t group,
		uint32_t ignoreClusterId,
		AlignedVector< QueryResult >& outResult
	) const;

	virtual void queryOverlap(
		const Body* body,
		RefArray< Body >& outResult
	) const;

	virtual void queryTriangles(
		const Vector4& center,
		float radius,
		AlignedVector< TriangleResult >& outTriangles
	) const;

	virtual void getStatistics(PhysicsStatistics& outStatistics) const;

private:
	float m_simulationDeltaTime;
	hkPoolMemory* m_memoryManager;
	hkThreadMemory* m_threadMemory;
	uint8_t* m_stackBuffer;
	hkpWorld* m_world;
	RefArray< BodyHavok > m_bodies;
	RefArray< Joint > m_joints;

	virtual void destroyBody(Body* body, const HvkRef< hkpRigidBody >& rigidBody);

	virtual void destroyJoint(Joint* joint, const HvkRef< hkpConstraintInstance >& constraint);

	static void showError(const char* error, void*);
};

	}
}

#endif	// traktor_physics_PhysicsManagerHavok_H
