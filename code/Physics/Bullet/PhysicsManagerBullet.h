/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Thread/Semaphore.h"
#include "Physics/PhysicsManager.h"
#include "Physics/Bullet/Types.h"
#include "Resource/Proxy.h"

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

namespace traktor::physics
{

class BodyBullet;
class Joint;
class ShapeDesc;

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

	virtual bool create(const PhysicsCreateDesc& desc) override final;

	virtual void destroy() override final;

	virtual void setGravity(const Vector4& gravity) override final;

	virtual Vector4 getGravity() const override final;

	virtual Ref< Body > createBody(resource::IResourceManager* resourceManager, const BodyDesc* desc, const wchar_t* const tag) override final;

	virtual Ref< Body > createBody(resource::IResourceManager* resourceManager, const BodyDesc* desc, const Mesh* mesh, const wchar_t* const tag) override final;

	virtual Ref< Joint > createJoint(const JointDesc* desc, const Transform& transform, Body* body1, Body* body2) override final;

	virtual void update(float simulationDeltaTime, bool issueCollisionEvents) override final;

	virtual void solveConstraints(const RefArray< Body >& bodies, const RefArray< Joint >& joints) override final;

	virtual RefArray< Body > getBodies() const override final;

	virtual uint32_t getCollidingPairs(std::vector< CollisionPair >& outCollidingPairs) const override final;

	virtual bool queryPoint(
		const Vector4& at,
		float margin,
		QueryResult& outResult
	) const override final;

	virtual bool queryRay(
		const Vector4& at,
		const Vector4& direction,
		float maxLength,
		const QueryFilter& queryFilter,
		bool ignoreBackFace,
		QueryResult& outResult
	) const override final;

	virtual bool queryShadowRay(
		const Vector4& at,
		const Vector4& direction,
		float maxLength,
		const QueryFilter& queryFilter,
		uint32_t queryTypes
	) const override final;

	virtual uint32_t querySphere(
		const Vector4& at,
		float radius,
		const QueryFilter& queryFilter,
		uint32_t queryTypes,
		RefArray< Body >& outBodies
	) const override final;

	virtual bool querySweep(
		const Vector4& at,
		const Vector4& direction,
		float maxLength,
		float radius,
		const QueryFilter& queryFilter,
		QueryResult& outResult
	) const override final;

	virtual bool querySweep(
		const Body* body,
		const Quaternion& orientation,
		const Vector4& at,
		const Vector4& direction,
		float maxLength,
		const QueryFilter& queryFilter,
		QueryResult& outResult
	) const override final;

	virtual void querySweep(
		const Vector4& at,
		const Vector4& direction,
		float maxLength,
		float radius,
		const QueryFilter& queryFilter,
		AlignedVector< QueryResult >& outResult
	) const override final;

	virtual void queryOverlap(
		const Body* body,
		RefArray< Body >& outResult
	) const override final;

	void queryTriangles(
		const Vector4& center,
		float radius,
		AlignedVector< TriangleResult >& outTriangles
	) const override final;

	virtual void getStatistics(PhysicsStatistics& outStatistics) const override final;

private:
	float m_timeScale = 1.0f;
	float m_simulationFrequency = 120.0f;
	Semaphore m_lock;
	btCollisionConfiguration* m_configuration;
	btCollisionDispatcher* m_dispatcher;
	btBroadphaseInterface* m_broadphase;
	btConstraintSolver* m_solver;
	btDiscreteDynamicsWorld* m_dynamicsWorld;
	RefArray< BodyBullet > m_bodies;
	RefArray< Joint > m_joints;
	uint32_t m_queryCountLast;
	mutable uint32_t m_queryCount;

	static PhysicsManagerBullet* ms_this;

	Ref< Body > createBody(resource::IResourceManager* resourceManager, const ShapeDesc* shapeDesc, const BodyDesc* desc, btCollisionShape* shape, const wchar_t* const tag, Vector4 centerOfGravity, const resource::Proxy< Mesh >& mesh);

	static void nearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo);

	virtual void insertBody(btRigidBody* rigidBody, uint16_t collisionGroup, uint16_t collisionFilter) override final;

	virtual void removeBody(btRigidBody* rigidBody) override final;

	virtual void insertConstraint(btTypedConstraint* constraint) override final;

	virtual void removeConstraint(btTypedConstraint* constraint) override final;

	virtual void destroyBody(BodyBullet* body, btRigidBody* rigidBody, btCollisionShape* shape) override final;

	virtual void destroyConstraint(Joint* joint, btTypedConstraint* constraint) override final;
};

}
