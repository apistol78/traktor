/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Transform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::physics
{

struct CollisionInfo;
class CollisionListener;

class BodyDesc;
class Body;
class JointDesc;
class Joint;
class Mesh;

/*! Ray query result.
 * \ingroup Physics
 */
struct QueryResult
{
	Ref< Body > body;
	Vector4 position = Vector4::zero();
	Vector4 normal = Vector4::zero();
	float distance = 0.0f;
	float fraction = 0.0f;
	int32_t material = 0;
};

/*! Triangle result-
 * \ingroup Physics
 */
struct TriangleResult
{
	Vector4 v[3];
};

/*! Collision pair.
 * \ingroup Physics
 */
struct CollisionPair
{
	Ref< Body > body1;
	Ref< Body > body2;
};

/*! Create configuration.
 * \ingroup Physics
 */
struct PhysicsCreateDesc
{
	float timeScale = 1.0;
	float simulationFrequency = 120.0f;	//!< Simulation frequency, default 120 Hz which is twice per default game update.
	int32_t solverIterations = 8;		//!< Collision solver iterations.
};

/*! Runtime statistics.
 * \ingroup Physics
 */
struct PhysicsStatistics
{
	uint32_t bodyCount;
	uint32_t activeCount;
	uint32_t manifoldCount;
	uint32_t queryCount;
};

/*! Query filter.
 * \ingroup Physics
 *
 * To be included in query following rule must be satisfied:
 * (body.group & query.includeGroup != 0) && (body.group & query.ignoreGroup == 0) && (body.clusterId != query.ignoreClusterId)
 */
struct QueryFilter
{
	uint32_t includeGroup = ~0U;
	uint32_t ignoreGroup = 0;
	uint32_t ignoreClusterId = 0;

	QueryFilter() = default;

	explicit QueryFilter(uint32_t includeGroup_)
	:	includeGroup(includeGroup_)
	{
	}

	explicit QueryFilter(uint32_t includeGroup_, uint32_t ignoreGroup_)
	:	includeGroup(includeGroup_)
	,	ignoreGroup(ignoreGroup_)
	{
	}

	explicit QueryFilter(uint32_t includeGroup_, uint32_t ignoreGroup_, uint32_t ignoreClusterId_)
	:	includeGroup(includeGroup_)
	,	ignoreGroup(ignoreGroup_)
	,	ignoreClusterId(ignoreClusterId_)
	{
	}

	static QueryFilter onlyIgnoreClusterId(uint32_t ignoreClusterId)
	{
		return QueryFilter(~0U, 0U, ignoreClusterId);
	}
};

/*! Physics manager.
 * \ingroup Physics
 */
class T_DLLCLASS PhysicsManager : public Object
{
	T_RTTI_CLASS;

public:
	enum QueryType
	{
		QtStatic = 1,
		QtDynamic = 2,
		QtAll = QtStatic | QtDynamic
	};

	/*! Add collision listener.
	 *
	 * Collision listeners are invoked when
	 * there a near-collision between two rigid
	 * bodies are detected.
	 *
	 * \param collisionListener Collision listener instance.
	 */
	void addCollisionListener(CollisionListener* collisionListener);

	/*! Remove collision listener.
	 *
	 * Remove previously registered collision listener.
	 *
	 * \param collisionListener Collision listener instance.
	 */
	void removeCollisionListener(CollisionListener* collisionListener);

	/*! Notify collision listeners.
	 *
	 * Called by physics manager to notify
	 * registered collision listeners about
	 * detected near-collisions.
	 *
	 * \param collisionInfo Information about collision.
	 */
	void notifyCollisionListeners(const CollisionInfo& collisionInfo);

	/*! Return true if any collision listener registered.
	 *
	 * \return True if collision listener registered.
	 */
	bool haveCollisionListeners() const;

	/*! Create physics manager.
	 *
	 * \param desc Physics configuration.
	 */
	virtual bool create(const PhysicsCreateDesc& desc) = 0;

	/*! Destroy physics manager.
	 *
	 * Destroy physics manager including all
	 * rigid bodies.
	 */
	virtual void destroy() = 0;

	/*! Set global gravity vector.
	 *
	 * \param gravity Gravity vector.
	 */
	virtual void setGravity(const Vector4& gravity) = 0;

	/*! Get global gravity vector.
	 *
	 * \return Gravity vector.
	 */
	virtual Vector4 getGravity() const = 0;

	/*! Create rigid body.
	 *
	 * \note Created body are initially disabled; need to call setEnable(true)
	 * in order to add to simulation.
	 *
	 * \param resourceManager Resource manager.
	 * \param desc Rigid body description.
	 * \param tag Optional name tag of body; only for debugging.
	 * \return Rigid body instance.
	 */
	virtual Ref< Body > createBody(resource::IResourceManager* resourceManager, const BodyDesc* desc, const wchar_t* const tag = nullptr) = 0;

	/*! Create rigid body from an explicit mesh.
	 *
	 * \note Created body are initially disabled; need to call setEnable(true)
	 * in order to add to simulation.
	 *
	 * \param resourceManager Resource manager.
	 * \param desc Rigid body description.
	 * \param mesh Mesh shape.
	 * \param tag Optional name tag of body; only for debugging.
	 * \return Rigid body instance.
	 */
	virtual Ref< Body > createBody(resource::IResourceManager* resourceManager, const BodyDesc* desc, const Mesh* mesh, const wchar_t* const tag = nullptr) = 0;

	/*! Create joint between bodies.
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

	/*! Update simulation.
	 *
	 * Update entire simulation of the world,
	 * including physics, collision and joints.
	 * Time step is fixed and set when the physics
	 * manager is created.
	 *
	 * \param simulationDeltaTime Simulate delta time.
	 * \param issueCollisionEvents Issue listeners for new collision events.
	 */
	virtual void update(float simulationDeltaTime, bool issueCollisionEvents) = 0;

	/*! Solve joint constraints applied to given bodies.
	 */
	virtual void solveConstraints(const RefArray< Body >& bodies, const RefArray< Joint >& joints) = 0;

	/*! Get bodies.
	 */
	virtual RefArray< Body > getBodies() const = 0;

	/*! Get colliding pairs.
	 *
	 * Get all colliding pairs of bodies.
	 *
	 * \param outCollidingPairs Colliding pairs.
	 * \return Number of pairs.
	 */
	virtual uint32_t getCollidingPairs(std::vector< CollisionPair >& outCollidingPairs) const = 0;

	/*! Get closest exit boundary.
	 *
	 * Check if point is within collision shape and
	 * return an approximate closest exit direction and distance.
	 *
	 * \param at Point position.
	 * \param margin Collision margin.
	 * \param outResult Query result.
	 * \return True if point is in collision shape and result was found.
	 */
	virtual bool queryPoint(
		const Vector4& at,
		float margin,
		QueryResult& outResult
	) const = 0;

	/*! Ray cast world.
	 *
	 * Cast ray into the world and find closest
	 * intersection.
	 *
	 * \param at Ray origin in world space.
	 * \param direction Ray direction in world space.
	 * \param maxLength Maximum length of ray.
	 * \param queryFilter Query group and cluster filter.
	 * \param ignoreBackFace Ignore intersection with back-facing surfaces.
	 * \param outResult Intersection result, only modified if method returns true.
	 * \return True if intersection found.
	 */
	virtual bool queryRay(
		const Vector4& at,
		const Vector4& direction,
		float maxLength,
		const QueryFilter& queryFilter,
		bool ignoreBackFace,
		QueryResult& outResult
	) const = 0;

	/*! "Shadow" ray cast world.
	 *
	 * Cast ray into the world and check if any
	 * intersection.
	 *
	 * \param at Ray origin in world space.
	 * \param direction Ray direction in world space.
	 * \param maxLength Maximum length of ray.
	 * \param queryFilter Query group and cluster filter.
	 * \param queryTypes Type of bodies, @sa QueryTypes
	 * \return True if any intersection found.
	 */
	virtual bool queryShadowRay(
		const Vector4& at,
		const Vector4& direction,
		float maxLength,
		const QueryFilter& queryFilter,
		uint32_t queryTypes
	) const = 0;

	/*! Get all bodies within a sphere.
	 *
	 * \param at Sphere origin in world space.
	 * \param radius Sphere radius.
	 * \param queryFilter Query group and cluster filter.
	 * \param queryTypes Type of bodies, @sa QueryTypes
	 * \param outBodies Array of intersecting bodies.
	 * \return Number of bodies found.
	 */
	virtual uint32_t querySphere(
		const Vector4& at,
		float radius,
		const QueryFilter& queryFilter,
		uint32_t queryTypes,
		RefArray< Body >& outBodies
	) const = 0;

	/*! Get closest contact from a swept sphere.
	 *
	 * \param at Sphere origin in world space.
	 * \param direction Sweep direction in world space.
	 * \param maxLength Max sweep length.
	 * \param radius Sphere radius.
	 * \param queryFilter Query group and cluster filter.
	 * \param outResult Intersection result, only modified if method returns true.
	 * \return True if intersection found.
	 */
	virtual bool querySweep(
		const Vector4& at,
		const Vector4& direction,
		float maxLength,
		float radius,
		const QueryFilter& queryFilter,
		QueryResult& outResult
	) const = 0;

	/*! Get closest contact from a swept shape.
	 *
	 * \param body Sweeping body; using body's shape when sweeping.
	 * \param orientation Shape orientation when sweeping.
	 * \param at Shape origin in world space.
	 * \param direction Sweep direction in world space.
	 * \param maxLength Max sweep length.
	 * \param queryFilter Query group and cluster filter.
	 * \param outResult Intersection result, only modified if method returns true.
	 * \return True if intersection found.
	 */
	virtual bool querySweep(
		const Body* body,
		const Quaternion& orientation,
		const Vector4& at,
		const Vector4& direction,
		float maxLength,
		const QueryFilter& queryFilter,
		QueryResult& outResult
	) const = 0;

	/*! Get all contact bodies from swept sphere.
	 *
	 * \param at Sphere origin in world space.
	 * \param direction Sweep direction in world space.
	 * \param maxLength Max sweep length.
	 * \param radius Sphere radius.
	 * \param queryFilter Query group and cluster filter.
	 * \param outResult Overlapping bodies result.
	 */
	virtual void querySweep(
		const Vector4& at,
		const Vector4& direction,
		float maxLength,
		float radius,
		const QueryFilter& queryFilter,
		AlignedVector< QueryResult >& outResult
	) const = 0;

	/*! Get overlapping bodies.
	 *
	 * \param body Check body; using body's shape when performing query.
	 * \param outResult Overlapping bodies result.
	 */
	virtual void queryOverlap(
		const Body* body,
		RefArray< Body >& outResult
	) const = 0;

	/*! Get triangles inside sphere.
	 *
	 * \param center Query sphere center.
	 * \param radius Query sphere radius.
	 * \param outTriangles Found triangles.
	 */
	virtual void queryTriangles(
		const Vector4& center,
		float radius,
		AlignedVector< TriangleResult >& outTriangles
	) const = 0;

	/*! Get runtime statistics.
	 *
	 * This method is mostly used for debugging
	 * purposes to ensure bodies go to sleep etc.
	 */
	virtual void getStatistics(PhysicsStatistics& outStatistics) const = 0;

private:
	RefArray< CollisionListener > m_collisionListeners;
};

}
