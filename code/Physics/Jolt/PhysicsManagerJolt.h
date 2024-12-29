/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Misc/AutoPtr.h"
#include "Physics/PhysicsManager.h"
#include "Physics/Jolt/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_JOLT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace JPH
{

class BroadPhaseLayerInterface;
class ContactListener;
class JobSystemThreadPool;
class ObjectLayerPairFilter;
class ObjectVsBroadPhaseLayerFilter;
class PhysicsSystem;
class TempAllocatorImpl;

}

namespace traktor::physics
{

class BodyJolt;
class Joint;
class ShapeDesc;

/*!
 * \ingroup Jolt
 */
class T_DLLCLASS PhysicsManagerJolt : public PhysicsManager, public IWorldCallback
{
	T_RTTI_CLASS;

public:
	virtual ~PhysicsManagerJolt();

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
	AutoPtr< JPH::TempAllocatorImpl > m_tempAllocator;
	AutoPtr< JPH::JobSystemThreadPool > m_jobSystem;
	AutoPtr< JPH::BroadPhaseLayerInterface > m_broadPhaseLayerInterface;
	AutoPtr< JPH::ObjectVsBroadPhaseLayerFilter > m_objectVsBroadPhaseLayerFilter;
	AutoPtr< JPH::ObjectLayerPairFilter > m_objectVsObjectLayerFilter;
	AutoPtr< JPH::ContactListener > m_contactListener;
	AutoPtr< JPH::PhysicsSystem > m_physicsSystem;
	RefArray< BodyJolt > m_bodies;
	float m_timeScale = 1.0f;

	Ref< Body > createBody(resource::IResourceManager* resourceManager, const BodyDesc* desc, const Mesh* mesh, uint32_t collisionGroup, uint32_t collisionMask, const wchar_t* const tag);

	// IWorldCallback

	virtual void destroyBody(BodyJolt* body) override final;
};

}
