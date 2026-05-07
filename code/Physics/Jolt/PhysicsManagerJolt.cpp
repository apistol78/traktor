/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/Jolt/PhysicsManagerJolt.h"

#include "Core/Log/Log.h"
#include "Core/Math/Aabb3.h"
#include "Heightfield/Heightfield.h"
#include "Physics/AxisJoint.h"
#include "Physics/AxisJointDesc.h"
#include "Physics/BallJoint.h"
#include "Physics/BallJointDesc.h"
#include "Physics/BoxShapeDesc.h"
#include "Physics/CapsuleShapeDesc.h"
#include "Physics/CollisionListener.h"
#include "Physics/CollisionSpecification.h"
#include "Physics/ConeTwistJoint.h"
#include "Physics/ConeTwistJointDesc.h"
#include "Physics/CylinderShapeDesc.h"
#include "Physics/DofJoint.h"
#include "Physics/DofJointDesc.h"
#include "Physics/DynamicBodyDesc.h"
#include "Physics/HeightfieldShapeDesc.h"
#include "Physics/Hinge2Joint.h"
#include "Physics/Hinge2JointDesc.h"
#include "Physics/HingeJoint.h"
#include "Physics/HingeJointDesc.h"
#include "Physics/Jolt/AxisJointJolt.h"
#include "Physics/Jolt/BallJointJolt.h"
#include "Physics/Jolt/BodyJolt.h"
#include "Physics/Jolt/ConeTwistJointJolt.h"
#include "Physics/Jolt/Conversion.h"
#include "Physics/Jolt/DofJointJolt.h"
#include "Physics/Jolt/Hinge2JointJolt.h"
#include "Physics/Jolt/HingeJointJolt.h"
#include "Physics/Mesh.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/SphereShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Resource/IResourceManager.h"

#include <algorithm>
#include <cstring>
#include <mutex>
#include <thread>
#include <unordered_map>

// Keep Jolt includes here, Jolt.h must be first.
#include <Jolt/Jolt.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/CollisionGroup.h>
#include <Jolt/Physics/Collision/GroupFilter.h>
#include <Jolt/Physics/Collision/PhysicsMaterialSimple.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Constraints/HingeConstraint.h>
#include <Jolt/Physics/Constraints/PointConstraint.h>
#include <Jolt/Physics/Constraints/SixDOFConstraint.h>
#include <Jolt/Physics/Constraints/SwingTwistConstraint.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

using namespace JPH::literals;

namespace traktor::physics
{
namespace
{

namespace Layers
{
constexpr JPH::ObjectLayer NON_MOVING = 0;
constexpr JPH::ObjectLayer MOVING = 1;
constexpr JPH::ObjectLayer NUM_LAYERS = 2;
}

namespace BroadPhaseLayers
{
constexpr JPH::BroadPhaseLayer NON_MOVING(0);
constexpr JPH::BroadPhaseLayer MOVING(1);
constexpr JPH::uint NUM_LAYERS(2);
}

class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
{
public:
	virtual bool ShouldCollide(JPH::ObjectLayer object1, JPH::ObjectLayer object2) const override
	{
		switch (object1)
		{
		case Layers::NON_MOVING:
			return object2 == Layers::MOVING;
		case Layers::MOVING:
			return true;
		default:
			return false;
		}
	}
};

class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
	BPLayerInterfaceImpl()
	{
		m_objectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
		m_objectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
	}

	virtual JPH::uint GetNumBroadPhaseLayers() const override
	{
		return BroadPhaseLayers::NUM_LAYERS;
	}

	virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override
	{
		T_ASSERT(layer < Layers::NUM_LAYERS);
		return m_objectToBroadPhase[layer];
	}

private:
	JPH::BroadPhaseLayer m_objectToBroadPhase[Layers::NUM_LAYERS];
};

class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
	virtual bool ShouldCollide(JPH::ObjectLayer layer1, JPH::BroadPhaseLayer layer2) const override
	{
		switch (layer1)
		{
		case Layers::NON_MOVING:
			return layer2 == BroadPhaseLayers::MOVING;
		case Layers::MOVING:
			return true;
		default:
			return false;
		}
	}
};

// Filters by traktor's collision group/mask bitmasks. The body's GroupID stores
// the merged collision-group bits, the SubGroupID stores the merged collision-mask
// bits. Two bodies collide only if each appears in the other's mask.
class GroupFilterImpl : public JPH::GroupFilter
{
public:
	virtual bool CanCollide(const JPH::CollisionGroup& group1, const JPH::CollisionGroup& group2) const override
	{
		const uint32_t g1 = group1.GetGroupID();
		const uint32_t m1 = group1.GetSubGroupID();
		const uint32_t g2 = group2.GetGroupID();
		const uint32_t m2 = group2.GetSubGroupID();
		return (g1 & m2) != 0 && (g2 & m1) != 0;
	}
};

class ContactListenerImpl : public JPH::ContactListener
{
public:
	explicit ContactListenerImpl(PhysicsManagerJolt* manager)
		: m_manager(manager)
	{
	}

	virtual JPH::ValidateResult OnContactValidate(const JPH::Body& body1, const JPH::Body& body2, JPH::RVec3Arg, const JPH::CollideShapeResult&) override
	{
		const BodyJolt* b1 = (const BodyJolt*)body1.GetUserData();
		const BodyJolt* b2 = (const BodyJolt*)body2.GetUserData();
		if (b1 && b2)
		{
			// Cluster id: bodies in the same valid cluster never collide.
			const uint32_t c1 = b1->getClusterId();
			if (c1 != ~0U && c1 == b2->getClusterId())
				return JPH::ValidateResult::RejectAllContactsForThisBodyPair;
		}
		return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
	}

	virtual void OnContactAdded(const JPH::Body& body1, const JPH::Body& body2, const JPH::ContactManifold& manifold, JPH::ContactSettings& ioSettings) override
	{
		BodyJolt* b1 = (BodyJolt*)body1.GetUserData();
		BodyJolt* b2 = (BodyJolt*)body2.GetUserData();
		if (!b1 || !b2)
			return;

		// Combine per-triangle friction and restitution from both bodies.
		float friction1 = body1.GetFriction(), restitution1 = body1.GetRestitution();
		float friction2 = body2.GetFriction(), restitution2 = body2.GetRestitution();
		b1->getFrictionAndRestitution(manifold.mSubShapeID1, friction1, restitution1);
		b2->getFrictionAndRestitution(manifold.mSubShapeID2, friction2, restitution2);

		ioSettings.mCombinedFriction = friction1 * friction2;
		ioSettings.mCombinedRestitution = restitution1 * restitution2;

		std::lock_guard< std::mutex > lock(m_mutex);

		// Track active pair.
		const uint64_t key = makeKey(body1.GetID(), body2.GetID());
		m_pairs[key] = ActivePair{ b1, b2 };

		// Queue collision event.
		PendingContact pc;
		pc.body1 = b1;
		pc.body2 = b2;
		pc.position = convertFromJolt(manifold.GetWorldSpaceContactPointOn1(0), 1.0f);
		pc.normal = convertFromJolt(manifold.mWorldSpaceNormal, 0.0f);
		pc.depth = manifold.mPenetrationDepth;
		m_pending.push_back(pc);
	}

	virtual void OnContactPersisted(const JPH::Body&, const JPH::Body&, const JPH::ContactManifold&, JPH::ContactSettings&) override {}

	virtual void OnContactRemoved(const JPH::SubShapeIDPair& subShapePair) override
	{
		std::lock_guard< std::mutex > lock(m_mutex);
		m_pairs.erase(makeKey(subShapePair.GetBody1ID(), subShapePair.GetBody2ID()));
	}

	struct PendingContact
	{
		BodyJolt* body1;
		BodyJolt* body2;
		Vector4 position;
		Vector4 normal;
		float depth;
	};

	struct ActivePair
	{
		BodyJolt* body1;
		BodyJolt* body2;
	};

	std::vector< PendingContact > drainPending()
	{
		std::lock_guard< std::mutex > lock(m_mutex);
		std::vector< PendingContact > out = std::move(m_pending);
		m_pending.clear();
		return out;
	}

	void getActivePairs(std::vector< CollisionPair >& outPairs) const
	{
		std::lock_guard< std::mutex > lock(m_mutex);
		outPairs.reserve(outPairs.size() + m_pairs.size());
		for (const auto& kv : m_pairs)
		{
			CollisionPair pair;
			pair.body1 = kv.second.body1;
			pair.body2 = kv.second.body2;
			outPairs.push_back(pair);
		}
	}

	uint32_t getActivePairCount() const
	{
		std::lock_guard< std::mutex > lock(m_mutex);
		return (uint32_t)m_pairs.size();
	}

	void onBodyDestroyed(BodyJolt* body)
	{
		std::lock_guard< std::mutex > lock(m_mutex);
		for (auto it = m_pairs.begin(); it != m_pairs.end(); )
		{
			if (it->second.body1 == body || it->second.body2 == body)
				it = m_pairs.erase(it);
			else
				++it;
		}
		for (auto it = m_pending.begin(); it != m_pending.end(); )
		{
			if (it->body1 == body || it->body2 == body)
				it = m_pending.erase(it);
			else
				++it;
		}
	}

private:
	static uint64_t makeKey(JPH::BodyID a, JPH::BodyID b)
	{
		const uint32_t ai = a.GetIndexAndSequenceNumber();
		const uint32_t bi = b.GetIndexAndSequenceNumber();
		return ai < bi
			? ((uint64_t)ai << 32) | (uint64_t)bi
			: ((uint64_t)bi << 32) | (uint64_t)ai;
	}

	PhysicsManagerJolt* m_manager;
	mutable std::mutex m_mutex;
	std::vector< PendingContact > m_pending;
	std::unordered_map< uint64_t, ActivePair > m_pairs;
};

bool resolveCollisionMask(
	resource::IResourceManager* resourceManager,
	const SmallSet< resource::Id< CollisionSpecification > >& ids,
	const wchar_t* const errorContext,
	uint32_t& outMask)
{
	outMask = 0;
	for (const auto& id : ids)
	{
		resource::Proxy< CollisionSpecification > spec;
		if (!resourceManager->bind(id, spec))
		{
			log::error << L"Unable to bind collision " << errorContext << L" specification." << Endl;
			return false;
		}
		outMask |= spec->getBitMask();
	}
	return true;
}

JPH::ShapeRefC wrapWithLocalTransform(JPH::ShapeSettings& shapeSettings, const Transform& localTransform)
{
	if (localTransform != Transform::identity())
	{
		JPH::MutableCompoundShapeSettings compoundSettings;
		compoundSettings.AddShape(
			convertToJolt(localTransform.translation()),
			convertToJolt(localTransform.rotation()),
			&shapeSettings);
		return compoundSettings.Create().Get();
	}
	return shapeSettings.Create().Get();
}

bool buildBodyCreationSettings(const BodyDesc* desc, JPH::ShapeRefC shape, JPH::BodyCreationSettings& outSettings)
{
	if (auto staticDesc = dynamic_type_cast< const StaticBodyDesc* >(desc))
	{
		const bool kinematic = staticDesc->isKinematic();
		outSettings = JPH::BodyCreationSettings(
			shape,
			JPH::RVec3::sZero(),
			JPH::Quat::sIdentity(),
			kinematic ? JPH::EMotionType::Kinematic : JPH::EMotionType::Static,
			kinematic ? Layers::MOVING : Layers::NON_MOVING);
		outSettings.mFriction = staticDesc->getFriction();
		outSettings.mRestitution = staticDesc->getRestitution();
		return true;
	}
	if (auto dynamicDesc = dynamic_type_cast< const DynamicBodyDesc* >(desc))
	{
		outSettings = JPH::BodyCreationSettings(
			shape,
			JPH::RVec3::sZero(),
			JPH::Quat::sIdentity(),
			JPH::EMotionType::Dynamic,
			Layers::MOVING);
		outSettings.mLinearDamping = dynamicDesc->getLinearDamping();
		outSettings.mAngularDamping = dynamicDesc->getAngularDamping();
		outSettings.mFriction = dynamicDesc->getFriction();
		outSettings.mRestitution = dynamicDesc->getRestitution();
		outSettings.mAllowSleeping = dynamicDesc->getAutoDeactivate();
		return true;
	}
	return false;
}

bool passesQueryFilter(const BodyJolt* body, const QueryFilter& filter)
{
	if (filter.ignoreClusterId != 0 && body->getClusterId() == filter.ignoreClusterId)
		return false;
	const uint32_t group = body->getCollisionGroup();
	if ((group & filter.includeGroup) == 0 || (group & filter.ignoreGroup) != 0)
		return false;
	return true;
}

bool passesQueryTypes(const BodyJolt* body, uint32_t queryTypes)
{
	const bool isStatic = body->isStatic() || body->isKinematic();
	if (isStatic && (queryTypes & PhysicsManager::QtStatic) == 0)
		return false;
	if (!isStatic && (queryTypes & PhysicsManager::QtDynamic) == 0)
		return false;
	return true;
}

class RayCollector : public JPH::CastRayCollector
{
public:
	RayCollector(const PhysicsManagerJolt* outer, const JPH::RRayCast& ray, const QueryFilter& queryFilter, uint32_t queryTypes, QueryResult& outResult)
		: m_outer(outer), m_ray(ray), m_queryFilter(queryFilter), m_queryTypes(queryTypes), m_outResult(outResult)
	{
	}

	virtual void AddHit(const JPH::RayCastResult& result) override
	{
		if (result.mFraction >= GetEarlyOutFraction())
			return;

		JPH::BodyLockRead lock(m_outer->getJPhysicsSystem()->GetBodyLockInterface(), result.mBodyID);
		if (!lock.Succeeded())
			return;

		const JPH::Body& hitBody = lock.GetBody();
		BodyJolt* unwrappedBody = (BodyJolt*)hitBody.GetUserData();
		if (!unwrappedBody || !passesQueryFilter(unwrappedBody, m_queryFilter) || !passesQueryTypes(unwrappedBody, m_queryTypes))
			return;

		const JPH::Vec3 position = m_ray.GetPointOnRay(result.mFraction);
		const JPH::Vec3 normal = hitBody.GetWorldSpaceSurfaceNormal(result.mSubShapeID2, position);

		m_outResult.body = unwrappedBody;
		m_outResult.position = convertFromJolt(position, 1.0f);
		m_outResult.normal = convertFromJolt(normal, 0.0f);
		m_outResult.fraction = result.mFraction;
		m_outResult.material = unwrappedBody->getMaterial();

		m_anyHit = true;
		UpdateEarlyOutFraction(result.mFraction);
	}

	bool AnyHit() const { return m_anyHit; }

private:
	const PhysicsManagerJolt* m_outer;
	const JPH::RRayCast& m_ray;
	const QueryFilter& m_queryFilter;
	uint32_t m_queryTypes;
	QueryResult& m_outResult;
	bool m_anyHit = false;
};

class SweepCollectorBase : public JPH::CastShapeCollector
{
public:
	SweepCollectorBase(const PhysicsManagerJolt* outer, const JPH::RShapeCast& shapeCast, const QueryFilter& queryFilter, const BodyJolt* ignoreBody)
		: m_outer(outer), m_shapeCast(shapeCast), m_queryFilter(queryFilter), m_ignoreBody(ignoreBody)
	{
	}

protected:
	bool resolveHit(const JPH::ShapeCastResult& result, QueryResult& outResult)
	{
		if (result.mFraction >= GetEarlyOutFraction())
			return false;

		JPH::BodyLockRead lock(m_outer->getJPhysicsSystem()->GetBodyLockInterface(), result.mBodyID2);
		if (!lock.Succeeded())
			return false;

		BodyJolt* unwrappedBody = (BodyJolt*)lock.GetBody().GetUserData();
		if (!unwrappedBody || unwrappedBody == m_ignoreBody || !passesQueryFilter(unwrappedBody, m_queryFilter))
			return false;

		outResult.body = unwrappedBody;
		outResult.position = convertFromJolt(m_shapeCast.GetPointOnRay(result.mFraction), 1.0f);
		outResult.normal = convertFromJolt(-result.mPenetrationAxis.Normalized(), 0.0f);
		outResult.fraction = result.mFraction;
		outResult.material = unwrappedBody->getMaterial();
		return true;
	}

	const PhysicsManagerJolt* m_outer;
	const JPH::RShapeCast& m_shapeCast;
	const QueryFilter& m_queryFilter;
	const BodyJolt* m_ignoreBody;
};

class SweepCollector final : public SweepCollectorBase
{
public:
	SweepCollector(const PhysicsManagerJolt* outer, const JPH::RShapeCast& shapeCast, const QueryFilter& queryFilter, QueryResult& outResult, const BodyJolt* ignoreBody = nullptr)
		: SweepCollectorBase(outer, shapeCast, queryFilter, ignoreBody), m_outResult(outResult)
	{
	}

	virtual void AddHit(const JPH::ShapeCastResult& result) override
	{
		if (resolveHit(result, m_outResult))
		{
			m_anyHit = true;
			UpdateEarlyOutFraction(result.mFraction);
		}
	}

	bool AnyHit() const { return m_anyHit; }

private:
	QueryResult& m_outResult;
	bool m_anyHit = false;
};

class SweepMultiCollector final : public SweepCollectorBase
{
public:
	SweepMultiCollector(const PhysicsManagerJolt* outer, const JPH::RShapeCast& shapeCast, const QueryFilter& queryFilter, AlignedVector< QueryResult >& outResult)
		: SweepCollectorBase(outer, shapeCast, queryFilter, nullptr), m_outResult(outResult)
	{
	}

	virtual void AddHit(const JPH::ShapeCastResult& result) override
	{
		QueryResult hit;
		if (resolveHit(result, hit))
		{
			m_outResult.push_back(hit);
			UpdateEarlyOutFraction(result.mFraction);
		}
	}

private:
	AlignedVector< QueryResult >& m_outResult;
};

class OverlapCollector : public JPH::CollideShapeCollector
{
public:
	OverlapCollector(const PhysicsManagerJolt* outer, const BodyJolt* ignoreBody, RefArray< Body >& outResult)
		: m_outer(outer), m_ignoreBody(ignoreBody), m_outResult(outResult)
	{
	}

	virtual void AddHit(const JPH::CollideShapeResult& result) override
	{
		JPH::BodyLockRead lock(m_outer->getJPhysicsSystem()->GetBodyLockInterface(), result.mBodyID2);
		if (!lock.Succeeded())
			return;
		BodyJolt* body = (BodyJolt*)lock.GetBody().GetUserData();
		if (!body || body == m_ignoreBody)
			return;
		m_outResult.push_back(body);
	}

private:
	const PhysicsManagerJolt* m_outer;
	const BodyJolt* m_ignoreBody;
	RefArray< Body >& m_outResult;
};

class SphereOverlapCollector : public JPH::CollideShapeBodyCollector
{
public:
	SphereOverlapCollector(const PhysicsManagerJolt* outer, const QueryFilter& queryFilter, uint32_t queryTypes, RefArray< Body >& outResult)
		: m_outer(outer), m_queryFilter(queryFilter), m_queryTypes(queryTypes), m_outResult(outResult)
	{
	}

	virtual void AddHit(const JPH::BodyID& bodyID) override
	{
		JPH::BodyLockRead lock(m_outer->getJPhysicsSystem()->GetBodyLockInterface(), bodyID);
		if (!lock.Succeeded())
			return;
		BodyJolt* body = (BodyJolt*)lock.GetBody().GetUserData();
		if (!body || !passesQueryFilter(body, m_queryFilter) || !passesQueryTypes(body, m_queryTypes))
			return;
		m_outResult.push_back(body);
	}

private:
	const PhysicsManagerJolt* m_outer;
	const QueryFilter& m_queryFilter;
	uint32_t m_queryTypes;
	RefArray< Body >& m_outResult;
};

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.PhysicsManagerJolt", 0, PhysicsManagerJolt, PhysicsManager)

PhysicsManagerJolt::~PhysicsManagerJolt()
{
	T_FATAL_ASSERT(m_physicsSystem.c_ptr() == nullptr);
}

bool PhysicsManagerJolt::create(const PhysicsCreateDesc& desc)
{
	// Hook our memory allocators to Jolt.
	JPH::Allocate = [](size_t size) {
		return Alloc::acquire(size, T_FILE_LINE);
	};
	JPH::Reallocate = [](void* block, size_t olds, size_t news) {
		void* newp = nullptr;
		if (news > 0)
		{
			newp = Alloc::acquire(news, T_FILE_LINE);
			std::memcpy(newp, block, std::min(olds, news));
		}
		Alloc::free(block);
		return newp;
	};
	JPH::Free = [](void* block) {
		Alloc::free(block);
	};
	JPH::AlignedAllocate = [](size_t size, size_t align) {
		return Alloc::acquireAlign(size, align, T_FILE_LINE);
	};
	JPH::AlignedFree = [](void* block) {
		Alloc::freeAlign(block);
	};

	JPH::Factory::sInstance = new JPH::Factory();
	JPH::RegisterTypes();

	m_tempAllocator.reset(new JPH::TempAllocatorImpl(32 * 1024 * 1024));
	m_jobSystem.reset(new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1));

	const JPH::uint cMaxBodies = 16384;
	const JPH::uint cNumBodyMutexes = 0;
	const JPH::uint cMaxBodyPairs = 16384;
	const JPH::uint cMaxContactConstraints = 4096;

	m_broadPhaseLayerInterface.reset(new BPLayerInterfaceImpl());
	m_objectVsBroadPhaseLayerFilter.reset(new ObjectVsBroadPhaseLayerFilterImpl());
	m_objectVsObjectLayerFilter.reset(new ObjectLayerPairFilterImpl());

	m_physicsSystem.reset(new JPH::PhysicsSystem);
	m_physicsSystem->Init(
		cMaxBodies,
		cNumBodyMutexes,
		cMaxBodyPairs,
		cMaxContactConstraints,
		*m_broadPhaseLayerInterface.ptr(),
		*m_objectVsBroadPhaseLayerFilter.ptr(),
		*m_objectVsObjectLayerFilter.ptr());

	{
		JPH::PhysicsSettings settings = m_physicsSystem->GetPhysicsSettings();
		settings.mNumPositionSteps = std::max(1, desc.solverIterations);
		m_physicsSystem->SetPhysicsSettings(settings);
	}

	m_contactListener.reset(new ContactListenerImpl(this));
	m_physicsSystem->SetContactListener(m_contactListener.ptr());

	m_groupFilter = new GroupFilterImpl();
	m_groupFilter->AddRef();

	m_physicsSystem->SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));

	m_timeScale = desc.timeScale;
	m_collisionSteps = std::max(1, (int32_t)(desc.simulationFrequency / 60.0f));
	return true;
}

void PhysicsManagerJolt::destroy()
{
	while (!m_joints.empty())
		m_joints.front()->destroy();
	while (!m_bodies.empty())
		m_bodies.front()->destroy();

	if (m_groupFilter)
	{
		m_groupFilter->Release();
		m_groupFilter = nullptr;
	}

	m_physicsSystem.release();
	m_contactListener.release();
	m_objectVsObjectLayerFilter.release();
	m_objectVsBroadPhaseLayerFilter.release();
	m_broadPhaseLayerInterface.release();
	m_jobSystem.release();
	m_tempAllocator.release();
}

void PhysicsManagerJolt::setGravity(const Vector4& gravity)
{
	m_physicsSystem->SetGravity(convertToJolt(gravity));
}

Vector4 PhysicsManagerJolt::getGravity() const
{
	return convertFromJolt(m_physicsSystem->GetGravity(), 0.0f);
}

Ref< Body > PhysicsManagerJolt::createBody(resource::IResourceManager* resourceManager, const BodyDesc* desc, const wchar_t* const tag)
{
	if (!desc)
		return nullptr;

	const ShapeDesc* shapeDesc = desc->getShape();
	if (!shapeDesc)
	{
		log::error << L"Unable to create body, no shape defined." << Endl;
		return nullptr;
	}

	uint32_t mergedCollisionGroup = 0;
	uint32_t mergedCollisionMask = 0;
	if (!resolveCollisionMask(resourceManager, shapeDesc->getCollisionGroup(), L"group", mergedCollisionGroup) ||
		!resolveCollisionMask(resourceManager, shapeDesc->getCollisionMask(), L"mask", mergedCollisionMask))
		return nullptr;

	if (auto boxShape = dynamic_type_cast< const BoxShapeDesc* >(shapeDesc))
	{
		JPH::BoxShapeSettings shapeSettings(convertToJolt(boxShape->getExtent()), boxShape->getMargin());
		shapeSettings.SetEmbedded();
		return createBodyFromShape(shapeSettings, shapeDesc, desc, 0.0f, Vector4::zero(), mergedCollisionGroup, mergedCollisionMask, resource::Proxy< Mesh >(), tag);
	}
	else if (auto sphereShape = dynamic_type_cast< const SphereShapeDesc* >(shapeDesc))
	{
		JPH::SphereShapeSettings shapeSettings(sphereShape->getRadius());
		shapeSettings.SetEmbedded();
		return createBodyFromShape(shapeSettings, shapeDesc, desc, 0.0f, Vector4::zero(), mergedCollisionGroup, mergedCollisionMask, resource::Proxy< Mesh >(), tag);
	}
	else if (auto capsuleShape = dynamic_type_cast< const CapsuleShapeDesc* >(shapeDesc))
	{
		const float radius = capsuleShape->getRadius();
		const float halfHeight = std::max(0.0f, capsuleShape->getLength() * 0.5f - radius);
		JPH::CapsuleShapeSettings shapeSettings(halfHeight, radius);
		shapeSettings.SetEmbedded();
		return createBodyFromShape(shapeSettings, shapeDesc, desc, 0.0f, Vector4::zero(), mergedCollisionGroup, mergedCollisionMask, resource::Proxy< Mesh >(), tag);
	}
	else if (auto cylinderShape = dynamic_type_cast< const CylinderShapeDesc* >(shapeDesc))
	{
		JPH::CylinderShapeSettings shapeSettings(cylinderShape->getLength() * 0.5f, cylinderShape->getRadius());
		shapeSettings.SetEmbedded();
		return createBodyFromShape(shapeSettings, shapeDesc, desc, 0.0f, Vector4::zero(), mergedCollisionGroup, mergedCollisionMask, resource::Proxy< Mesh >(), tag);
	}
	else if (auto meshShape = dynamic_type_cast< const MeshShapeDesc* >(shapeDesc))
	{
		resource::Proxy< Mesh > mesh;
		if (!resourceManager->bind(meshShape->getMesh(), mesh))
		{
			log::error << L"Unable to load collision mesh resource " << Guid(meshShape->getMesh()).format() << L"." << Endl;
			return nullptr;
		}
		return createBody(resourceManager, desc, mesh, mergedCollisionGroup, mergedCollisionMask, tag);
	}
	else if (auto heightfieldShape = dynamic_type_cast< const HeightfieldShapeDesc* >(shapeDesc))
	{
		resource::Proxy< hf::Heightfield > heightfield;
		if (!resourceManager->bind(heightfieldShape->getHeightfield(), heightfield))
		{
			log::error << L"Unable to load heightfield resource." << Endl;
			return nullptr;
		}

		const int32_t size = heightfield->getSize();
		AlignedVector< float > samples(size * size);
		for (int32_t y = 0; y < size; ++y)
			for (int32_t x = 0; x < size; ++x)
				samples[x + y * size] = heightfield->getGridHeightNearest(x, y);

		const Vector4& worldExtent = heightfield->getWorldExtent();
		const Vector4 scale(1.0f / size, 1.0f, 1.0f / size, 1.0f);

		JPH::HeightFieldShapeSettings shapeSettings(
			samples.c_ptr(),
			convertToJolt(-worldExtent * 0.5_simd),
			convertToJolt(worldExtent * scale),
			size);
		shapeSettings.SetEmbedded();

		return createBodyFromShape(shapeSettings, shapeDesc, desc, 0.0f, Vector4::zero(), mergedCollisionGroup, mergedCollisionMask, resource::Proxy< Mesh >(), tag);
	}

	log::error << L"Unsupported shape type \"" << type_name(shapeDesc) << L"\"." << Endl;
	return nullptr;
}

Ref< Body > PhysicsManagerJolt::createBody(resource::IResourceManager* resourceManager, const BodyDesc* desc, const Mesh* mesh, const wchar_t* const tag)
{
	uint32_t mergedCollisionGroup = 0;
	uint32_t mergedCollisionMask = 0;
	if (!resolveCollisionMask(resourceManager, desc->getShape()->getCollisionGroup(), L"group", mergedCollisionGroup) ||
		!resolveCollisionMask(resourceManager, desc->getShape()->getCollisionMask(), L"mask", mergedCollisionMask))
		return nullptr;

	return createBody(resourceManager, desc, mesh, mergedCollisionGroup, mergedCollisionMask, tag);
}

Ref< Joint > PhysicsManagerJolt::createJoint(const JointDesc* desc, const Transform& transform, Body* body1, Body* body2)
{
	if (!desc || !body1)
		return nullptr;

	BodyJolt* bj1 = checked_type_cast< BodyJolt* >(body1);
	BodyJolt* bj2 = checked_type_cast< BodyJolt* >(body2);
	JPH::Body* jb1 = bj1->getJBody();
	JPH::Body* jb2 = bj2 ? bj2->getJBody() : &JPH::Body::sFixedToWorld;

	Ref< Joint > joint;

	if (auto axisDesc = dynamic_type_cast< const AxisJointDesc* >(desc))
	{
		const Vector4 anchor = transform * axisDesc->getAnchor().xyz1();
		const Vector4 axis = (transform * axisDesc->getAxis().xyz0()).normalized();

		JPH::HingeConstraintSettings settings;
		settings.mSpace = JPH::EConstraintSpace::WorldSpace;
		settings.mPoint1 = settings.mPoint2 = convertToJolt(anchor);
		settings.mHingeAxis1 = settings.mHingeAxis2 = convertToJolt(axis);
		const JPH::Vec3 normal = convertToJolt(axis).GetNormalizedPerpendicular();
		settings.mNormalAxis1 = settings.mNormalAxis2 = normal;

		auto* constraint = static_cast< JPH::HingeConstraint* >(settings.Create(*jb1, *jb2));
		joint = new AxisJointJolt(this, constraint, bj1, bj2);
	}
	else if (auto ballDesc = dynamic_type_cast< const BallJointDesc* >(desc))
	{
		const Vector4 anchor = transform * ballDesc->getAnchor().xyz1();

		JPH::PointConstraintSettings settings;
		settings.mSpace = JPH::EConstraintSpace::WorldSpace;
		settings.mPoint1 = settings.mPoint2 = convertToJolt(anchor);

		auto* constraint = static_cast< JPH::PointConstraint* >(settings.Create(*jb1, *jb2));
		joint = new BallJointJolt(this, constraint, bj1, bj2);
	}
	else if (auto coneTwistDesc = dynamic_type_cast< const ConeTwistJointDesc* >(desc))
	{
		const Vector4 anchor = transform * coneTwistDesc->getAnchor().xyz1();
		const Vector4 twistAxis = (transform * coneTwistDesc->getTwistAxis().xyz0()).normalized();
		const Vector4 coneAxis = (transform * coneTwistDesc->getConeAxis().xyz0()).normalized();

		float coneAngle1, coneAngle2;
		coneTwistDesc->getConeAngles(coneAngle1, coneAngle2);
		const float twistAngle = coneTwistDesc->getTwistAngle();

		JPH::SwingTwistConstraintSettings settings;
		settings.mSpace = JPH::EConstraintSpace::WorldSpace;
		settings.mPosition1 = settings.mPosition2 = convertToJolt(anchor);
		settings.mTwistAxis1 = settings.mTwistAxis2 = convertToJolt(twistAxis);
		settings.mPlaneAxis1 = settings.mPlaneAxis2 = convertToJolt(coneAxis);
		settings.mNormalHalfConeAngle = coneAngle1 * 0.5f;
		settings.mPlaneHalfConeAngle = coneAngle2 * 0.5f;
		settings.mTwistMinAngle = -twistAngle * 0.5f;
		settings.mTwistMaxAngle = twistAngle * 0.5f;

		auto* constraint = static_cast< JPH::SwingTwistConstraint* >(settings.Create(*jb1, *jb2));
		joint = new ConeTwistJointJolt(this, constraint, bj1, bj2);
	}
	else if (auto dofDesc = dynamic_type_cast< const DofJointDesc* >(desc))
	{
		JPH::SixDOFConstraintSettings settings;
		settings.mSpace = JPH::EConstraintSpace::WorldSpace;
		settings.mPosition1 = settings.mPosition2 = convertToJolt(transform.translation());
		settings.mAxisX1 = settings.mAxisX2 = convertToJolt(transform * Vector4(1.0f, 0.0f, 0.0f, 0.0f));
		settings.mAxisY1 = settings.mAxisY2 = convertToJolt(transform * Vector4(0.0f, 1.0f, 0.0f, 0.0f));

		using EAxis = JPH::SixDOFConstraintSettings::EAxis;
		const auto applyAxis = [&](EAxis a, bool free) {
			if (free)
				settings.MakeFreeAxis(a);
			else
				settings.MakeFixedAxis(a);
		};
		const auto& tr = dofDesc->getTranslate();
		const auto& ro = dofDesc->getRotate();
		applyAxis(EAxis::TranslationX, tr.x);
		applyAxis(EAxis::TranslationY, tr.y);
		applyAxis(EAxis::TranslationZ, tr.z);
		applyAxis(EAxis::RotationX, ro.x);
		applyAxis(EAxis::RotationY, ro.y);
		applyAxis(EAxis::RotationZ, ro.z);

		auto* constraint = static_cast< JPH::SixDOFConstraint* >(settings.Create(*jb1, *jb2));
		joint = new DofJointJolt(this, constraint, bj1, bj2);
	}
	else if (auto hingeDesc = dynamic_type_cast< const HingeJointDesc* >(desc))
	{
		const Vector4 anchor = transform * hingeDesc->getAnchor().xyz1();
		const Vector4 axis = (transform * hingeDesc->getAxis().xyz0()).normalized();

		JPH::HingeConstraintSettings settings;
		settings.mSpace = JPH::EConstraintSpace::WorldSpace;
		settings.mPoint1 = settings.mPoint2 = convertToJolt(anchor);
		settings.mHingeAxis1 = settings.mHingeAxis2 = convertToJolt(axis);
		const JPH::Vec3 normal = convertToJolt(axis).GetNormalizedPerpendicular();
		settings.mNormalAxis1 = settings.mNormalAxis2 = normal;

		if (hingeDesc->getEnableLimits())
		{
			float minAngle, maxAngle;
			hingeDesc->getAngles(minAngle, maxAngle);
			settings.mLimitsMin = minAngle;
			settings.mLimitsMax = maxAngle;
		}

		auto* constraint = static_cast< JPH::HingeConstraint* >(settings.Create(*jb1, *jb2));
		joint = new HingeJointJolt(this, constraint, bj1, bj2);
	}
	else if (auto hinge2Desc = dynamic_type_cast< const Hinge2JointDesc* >(desc))
	{
		if (!bj2)
			return nullptr;

		const Vector4 anchor = transform * hinge2Desc->getAnchor().xyz1();
		const Vector4 axis1 = (transform * hinge2Desc->getAxis1().xyz0()).normalized();
		const Vector4 axis2 = (transform * hinge2Desc->getAxis2().xyz0()).normalized();

		using EAxis = JPH::SixDOFConstraintSettings::EAxis;

		JPH::SixDOFConstraintSettings settings;
		settings.mSpace = JPH::EConstraintSpace::WorldSpace;
		settings.mPosition1 = settings.mPosition2 = convertToJolt(anchor);
		settings.mAxisX1 = settings.mAxisX2 = convertToJolt(axis1);
		settings.mAxisY1 = settings.mAxisY2 = convertToJolt(axis2);

		settings.MakeFixedAxis(EAxis::TranslationX);
		settings.MakeFixedAxis(EAxis::TranslationZ);
		if (hinge2Desc->getSuspensionEnable())
			settings.SetLimitedAxis(EAxis::TranslationY, -1.0f, 1.0f);
		else
			settings.MakeFixedAxis(EAxis::TranslationY);

		settings.SetLimitedAxis(EAxis::RotationX, hinge2Desc->getLowStop(), hinge2Desc->getHighStop());
		settings.MakeFreeAxis(EAxis::RotationY);
		settings.MakeFixedAxis(EAxis::RotationZ);

		if (hinge2Desc->getSuspensionEnable())
		{
			JPH::SpringSettings spring;
			spring.mFrequency = hinge2Desc->getSuspensionStiffness();
			spring.mDamping = hinge2Desc->getSuspensionDamping();
			settings.mLimitsSpringSettings[EAxis::TranslationY] = spring;
		}

		auto* constraint = static_cast< JPH::SixDOFConstraint* >(settings.Create(*jb1, *jb2));
		joint = new Hinge2JointJolt(this, constraint, bj1, bj2);
	}

	if (!joint)
	{
		log::error << L"Unable to create joint; unknown joint type \"" << type_name(desc) << L"\"." << Endl;
		return nullptr;
	}

	m_joints.push_back(joint);
	return joint;
}

void PhysicsManagerJolt::update(float simulationDeltaTime, bool issueCollisionEvents)
{
	m_physicsSystem->Update(simulationDeltaTime * m_timeScale, m_collisionSteps, m_tempAllocator.ptr(), m_jobSystem.ptr());

	if (issueCollisionEvents)
	{
		auto* listener = const_cast< ContactListenerImpl* >(static_cast< const ContactListenerImpl* >(m_contactListener.c_ptr()));
		auto pending = listener->drainPending();
		const bool any = haveCollisionListeners();
		for (const auto& pc : pending)
		{
			if (!pc.body1 || !pc.body2)
				continue;
			const bool body1Listens = pc.body1->haveCollisionListeners();
			const bool body2Listens = pc.body2->haveCollisionListeners();
			if (!any && !body1Listens && !body2Listens)
				continue;

			CollisionInfo info;
			info.body1 = pc.body1;
			info.body2 = pc.body2;
			CollisionContact cc;
			cc.position = pc.position;
			cc.normal = pc.normal;
			cc.depth = pc.depth;
			cc.material1 = pc.body1->getMaterial();
			cc.material2 = pc.body2->getMaterial();
			info.contacts.push_back(cc);

			if (any)
				notifyCollisionListeners(info);
			if (body1Listens)
				pc.body1->notifyCollisionListeners(info);
			if (body2Listens)
				pc.body2->notifyCollisionListeners(info);
		}
	}
	else
	{
		auto* listener = const_cast< ContactListenerImpl* >(static_cast< const ContactListenerImpl* >(m_contactListener.c_ptr()));
		listener->drainPending();
	}

	m_queryCountLast = m_queryCount;
	m_queryCount = 0;
}

void PhysicsManagerJolt::solveConstraints(const RefArray< Body >& bodies, const RefArray< Joint >& joints)
{
}

RefArray< Body > PhysicsManagerJolt::getBodies() const
{
	return *(RefArray< Body >*)&m_bodies;
}

uint32_t PhysicsManagerJolt::getCollidingPairs(std::vector< CollisionPair >& outCollidingPairs) const
{
	auto* listener = const_cast< ContactListenerImpl* >(static_cast< const ContactListenerImpl* >(m_contactListener.c_ptr()));
	listener->getActivePairs(outCollidingPairs);
	return (uint32_t)outCollidingPairs.size();
}

bool PhysicsManagerJolt::queryPoint(const Vector4& at, float margin, QueryResult& outResult) const
{
	++m_queryCount;
	return false;
}

bool PhysicsManagerJolt::queryRay(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	const QueryFilter& queryFilter,
	bool ignoreBackFace,
	QueryResult& outResult) const
{
	++m_queryCount;

	const JPH::RRayCast ray{ convertToJolt(at), convertToJolt(direction * Scalar(maxLength)) };

	JPH::RayCastSettings settings;
	if (ignoreBackFace)
	{
		settings.mBackFaceModeTriangles = JPH::EBackFaceMode::IgnoreBackFaces;
		settings.mBackFaceModeConvex = JPH::EBackFaceMode::IgnoreBackFaces;
	}
	else
	{
		settings.mBackFaceModeTriangles = JPH::EBackFaceMode::CollideWithBackFaces;
		settings.mBackFaceModeConvex = JPH::EBackFaceMode::CollideWithBackFaces;
	}
	settings.mTreatConvexAsSolid = true;

	RayCollector collector(this, ray, queryFilter, QtAll, outResult);
	m_physicsSystem->GetNarrowPhaseQuery().CastRay(ray, settings, collector);

	if (!collector.AnyHit())
		return false;

	outResult.distance = dot3(outResult.position - at, direction);
	return true;
}

bool PhysicsManagerJolt::queryShadowRay(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	const QueryFilter& queryFilter,
	uint32_t queryTypes) const
{
	++m_queryCount;

	const JPH::RRayCast ray{ convertToJolt(at), convertToJolt(direction * Scalar(maxLength)) };

	JPH::RayCastSettings settings;
	settings.mBackFaceModeTriangles = JPH::EBackFaceMode::CollideWithBackFaces;
	settings.mBackFaceModeConvex = JPH::EBackFaceMode::CollideWithBackFaces;
	settings.mTreatConvexAsSolid = true;

	QueryResult dummy;
	RayCollector collector(this, ray, queryFilter, queryTypes, dummy);
	m_physicsSystem->GetNarrowPhaseQuery().CastRay(ray, settings, collector);

	return collector.AnyHit();
}

uint32_t PhysicsManagerJolt::querySphere(
	const Vector4& at,
	float radius,
	const QueryFilter& queryFilter,
	uint32_t queryTypes,
	RefArray< Body >& outBodies) const
{
	++m_queryCount;

	outBodies.resize(0);

	SphereOverlapCollector collector(this, queryFilter, queryTypes, outBodies);
	m_physicsSystem->GetBroadPhaseQuery().CollideSphere(convertToJolt(at), radius, collector);

	return (uint32_t)outBodies.size();
}

bool PhysicsManagerJolt::querySweep(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	float radius,
	const QueryFilter& queryFilter,
	QueryResult& outResult) const
{
	++m_queryCount;

	JPH::SphereShape sphere(radius);
	sphere.SetEmbedded();

	const JPH::RShapeCast shapeCast(
		&sphere,
		JPH::Vec3::sReplicate(1.0f),
		JPH::RMat44::sTranslation(convertToJolt(at)),
		convertToJolt(direction * Scalar(maxLength)));

	JPH::ShapeCastSettings settings;
	settings.mUseShrunkenShapeAndConvexRadius = true;
	settings.mReturnDeepestPoint = true;

	SweepCollector collector(this, shapeCast, queryFilter, outResult);
	m_physicsSystem->GetNarrowPhaseQuery().CastShape(shapeCast, settings, JPH::Vec3::sZero(), collector);

	if (!collector.AnyHit())
		return false;

	outResult.distance = dot3(outResult.position - at, direction);
	return true;
}

bool PhysicsManagerJolt::querySweep(
	const Body* body,
	const Quaternion& orientation,
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	const QueryFilter& queryFilter,
	QueryResult& outResult) const
{
	++m_queryCount;

	const BodyJolt* bj = mandatory_non_null_type_cast< const BodyJolt* >(body);
	const JPH::Shape* shape = bj->getJBody()->GetShape();

	const JPH::RShapeCast shapeCast(
		shape,
		JPH::Vec3::sReplicate(1.0f),
		JPH::RMat44::sRotationTranslation(convertToJolt(orientation), convertToJolt(at)),
		convertToJolt(direction * Scalar(maxLength)));

	JPH::ShapeCastSettings settings;
	settings.mUseShrunkenShapeAndConvexRadius = true;
	settings.mReturnDeepestPoint = true;

	SweepCollector collector(this, shapeCast, queryFilter, outResult, bj);
	m_physicsSystem->GetNarrowPhaseQuery().CastShape(shapeCast, settings, JPH::Vec3::sZero(), collector);

	if (!collector.AnyHit())
		return false;

	outResult.distance = dot3(outResult.position - at, direction);
	return true;
}

void PhysicsManagerJolt::querySweep(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	float radius,
	const QueryFilter& queryFilter,
	AlignedVector< QueryResult >& outResult) const
{
	++m_queryCount;

	JPH::SphereShape sphere(radius);
	sphere.SetEmbedded();

	const JPH::RShapeCast shapeCast(
		&sphere,
		JPH::Vec3::sReplicate(1.0f),
		JPH::RMat44::sTranslation(convertToJolt(at)),
		convertToJolt(direction * Scalar(maxLength)));

	JPH::ShapeCastSettings settings;
	settings.mUseShrunkenShapeAndConvexRadius = true;
	settings.mReturnDeepestPoint = true;

	SweepMultiCollector collector(this, shapeCast, queryFilter, outResult);
	m_physicsSystem->GetNarrowPhaseQuery().CastShape(shapeCast, settings, JPH::Vec3::sZero(), collector);

	for (auto& result : outResult)
		result.distance = dot3(result.position - at, direction);
}

void PhysicsManagerJolt::queryOverlap(
	const Body* body,
	RefArray< Body >& outResult) const
{
	const BodyJolt* bj = mandatory_non_null_type_cast< const BodyJolt* >(body);
	const JPH::Shape* shape = bj->getJBody()->GetShape();
	const JPH::RMat44 transform = bj->getJBody()->GetWorldTransform();

	JPH::CollideShapeSettings settings;
	settings.mActiveEdgeMode = JPH::EActiveEdgeMode::CollideOnlyWithActive;
	settings.mCollectFacesMode = JPH::ECollectFacesMode::NoFaces;

	OverlapCollector collector(this, bj, outResult);
	m_physicsSystem->GetNarrowPhaseQuery().CollideShape(
		shape,
		JPH::Vec3::sReplicate(1.0f),
		transform,
		settings,
		JPH::Vec3::sZero(),
		collector);
}

void PhysicsManagerJolt::queryTriangles(const Vector4& center, float radius, AlignedVector< TriangleResult >& outTriangles) const
{
	const JPH::Vec3 jcenter = convertToJolt(center);
	const JPH::AABox worldBox(jcenter - JPH::Vec3::sReplicate(radius), jcenter + JPH::Vec3::sReplicate(radius));

	RefArray< Body > overlapping;
	{
		QueryFilter f;
		SphereOverlapCollector collector(this, f, QtAll, overlapping);
		m_physicsSystem->GetBroadPhaseQuery().CollideSphere(jcenter, radius, collector);
	}

	for (Body* b : overlapping)
	{
		const BodyJolt* bj = static_cast< const BodyJolt* >(b);
		const JPH::Body* jbody = bj->getJBody();
		const JPH::Shape* shape = jbody->GetShape();
		if (!shape)
			continue;

		const JPH::RMat44 transform = jbody->GetWorldTransform();

		// Local-space box matching the world AABB.
		const JPH::RMat44 inv = transform.InversedRotationTranslation();
		const JPH::Vec3 localCenter = JPH::Vec3(inv * jcenter);
		const JPH::AABox localBox(localCenter - JPH::Vec3::sReplicate(radius), localCenter + JPH::Vec3::sReplicate(radius));

		JPH::Shape::GetTrianglesContext ctx;
		shape->GetTrianglesStart(ctx, localBox, JPH::Vec3::sZero(), JPH::Quat::sIdentity(), JPH::Vec3::sReplicate(1.0f));

		const int cBatch = 64;
		JPH::Float3 verts[cBatch * 3];
		while (true)
		{
			const int n = shape->GetTrianglesNext(ctx, cBatch, verts);
			if (n <= 0)
				break;
			for (int i = 0; i < n; ++i)
			{
				TriangleResult tr;
				for (int v = 0; v < 3; ++v)
				{
					const JPH::Float3& p = verts[i * 3 + v];
					tr.v[v] = convertFromJolt(JPH::Vec3(transform * JPH::Vec3(p.x, p.y, p.z)), 1.0f);
				}
				outTriangles.push_back(tr);
			}
		}
	}
}

void PhysicsManagerJolt::getStatistics(PhysicsStatistics& outStatistics) const
{
	auto* listener = const_cast< ContactListenerImpl* >(static_cast< const ContactListenerImpl* >(m_contactListener.c_ptr()));
	outStatistics.bodyCount = (uint32_t)m_bodies.size();
	outStatistics.activeCount = (uint32_t)m_physicsSystem->GetNumActiveBodies(JPH::EBodyType::RigidBody);
	outStatistics.manifoldCount = listener->getActivePairCount();
	outStatistics.queryCount = m_queryCountLast;
}

Ref< Body > PhysicsManagerJolt::createBody(resource::IResourceManager* resourceManager, const BodyDesc* desc, const Mesh* mesh, uint32_t collisionGroup, uint32_t collisionMask, const wchar_t* const tag)
{
	const ShapeDesc* shapeDesc = desc->getShape();
	if (!shapeDesc)
	{
		log::error << L"Unable to create body, no shape defined." << Endl;
		return nullptr;
	}

	const Vector4 centerOfGravity = mesh->getOffset();
	const resource::Proxy< Mesh > meshProxy(const_cast< Mesh* >(mesh));

	if (auto staticDesc = dynamic_type_cast< const StaticBodyDesc* >(desc))
	{
		const auto& vertices = mesh->getVertices();
		const auto& triangles = mesh->getShapeTriangles();

		JPH::VertexList vertexList;
		vertexList.reserve(vertices.size());
		for (const auto& vertex : vertices)
			vertexList.push_back(JPH::Float3(vertex.x(), vertex.y(), vertex.z()));

		const uint32_t materialCount = (uint32_t)mesh->getMaterials().size();

		JPH::IndexedTriangleList triangleList;
		triangleList.reserve(triangles.size());
		for (const auto& triangle : triangles)
			triangleList.push_back(JPH::IndexedTriangle(
				triangle.indices[2],
				triangle.indices[1],
				triangle.indices[0],
				triangle.material < materialCount ? triangle.material : 0));

		// Jolt requires mMaterials to be populated for any non-zero per-triangle
		// material index. The actual friction/restitution come from the contact
		// callback (which reads our Mesh::Material table), so we just hand Jolt
		// a parallel list of placeholder PhysicsMaterials of the right size.
		JPH::PhysicsMaterialList joltMaterials;
		joltMaterials.reserve(materialCount);
		for (uint32_t i = 0; i < materialCount; ++i)
			joltMaterials.push_back(JPH::PhysicsMaterial::sDefault);

		JPH::MeshShapeSettings shapeSettings(vertexList, triangleList, std::move(joltMaterials));
		shapeSettings.SetEmbedded();

		return createBodyFromShape(shapeSettings, shapeDesc, desc, 0.0f, centerOfGravity, collisionGroup, collisionMask, meshProxy, tag);
	}
	else if (auto dynamicDesc = dynamic_type_cast< const DynamicBodyDesc* >(desc))
	{
		const auto& vertices = mesh->getVertices();
		const auto& hullIndices = mesh->getHullIndices();
		if (vertices.empty() || hullIndices.empty())
		{
			log::error << L"Unable to create body, mesh hull empty." << Endl;
			return nullptr;
		}

		JPH::Array< JPH::Vec3 > vertexList;
		vertexList.reserve(hullIndices.size());

		Aabb3 boundingBox;
		for (const auto& hullIndex : hullIndices)
		{
			const Vector4& vertex = vertices[hullIndex];
			vertexList.push_back(JPH::Vec3(vertex.x(), vertex.y(), vertex.z()));
			boundingBox.contain(vertex);
		}

		JPH::ConvexHullShapeSettings shapeSettings(vertexList);
		shapeSettings.SetEmbedded();

		JPH::ShapeRefC shape = wrapWithLocalTransform(shapeSettings, shapeDesc->getLocalTransform());

		JPH::BodyCreationSettings settings;
		if (!buildBodyCreationSettings(desc, shape, settings))
			return nullptr;

		const float mass = dynamicDesc->getMass();
		const Vector4 boxSize = boundingBox.getExtent() * 2.0_simd;
		const float volume = std::max< float >(boxSize.x() * boxSize.y() * boxSize.z(), 0.001f);

		settings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
		settings.mMassPropertiesOverride.SetMassAndInertiaOfSolidBox(convertToJolt(boxSize), mass / volume);
		settings.mCollisionGroup = JPH::CollisionGroup(m_groupFilter, collisionGroup, collisionMask);

		JPH::Body* body = m_physicsSystem->GetBodyInterface().CreateBody(settings);
		if (!body)
			return nullptr;

		Ref< BodyJolt > bj = new BodyJolt(tag, this, m_physicsSystem.ptr(), body, 1.0f / mass, centerOfGravity, collisionGroup, collisionMask, shapeDesc->getMaterial(), meshProxy);
		m_bodies.push_back(bj);
		return bj;
	}

	log::error << L"Unsupported body type for mesh shape." << Endl;
	return nullptr;
}

Ref< Body > PhysicsManagerJolt::createBodyFromShape(JPH::ShapeSettings& shapeSettings, const ShapeDesc* shapeDesc, const BodyDesc* desc, float inverseMass, const Vector4& centerOfGravity, uint32_t collisionGroup, uint32_t collisionMask, const resource::Proxy< Mesh >& mesh, const wchar_t* const tag)
{
	JPH::ShapeRefC shape = wrapWithLocalTransform(shapeSettings, shapeDesc->getLocalTransform());

	JPH::BodyCreationSettings settings;
	if (!buildBodyCreationSettings(desc, shape, settings))
		return nullptr;

	settings.mCollisionGroup = JPH::CollisionGroup(m_groupFilter, collisionGroup, collisionMask);

	JPH::Body* body = m_physicsSystem->GetBodyInterface().CreateBody(settings);
	if (!body)
		return nullptr;

	Ref< BodyJolt > bj = new BodyJolt(tag, this, m_physicsSystem.ptr(), body, inverseMass, centerOfGravity, collisionGroup, collisionMask, shapeDesc->getMaterial(), mesh);
	m_bodies.push_back(bj);
	return bj;
}

void PhysicsManagerJolt::destroyBody(BodyJolt* body)
{
	if (m_contactListener.c_ptr())
		static_cast< ContactListenerImpl* >(m_contactListener.ptr())->onBodyDestroyed(body);

	// Destroy joints which reference the body being destroyed.
	RefArray< Joint > joints = m_joints;
	for (auto joint : joints)
	{
		if (joint->getBody1() == body || joint->getBody2() == body)
			joint->destroy();
	}

	const bool removed = m_bodies.remove(body);
	T_FATAL_ASSERT(removed);
}

void PhysicsManagerJolt::destroyConstraint(Joint* joint, JPH::Constraint* constraint)
{
	if (m_physicsSystem.c_ptr())
		m_physicsSystem->RemoveConstraint(constraint);
	const bool removed = m_joints.remove(joint);
	T_FATAL_ASSERT(removed);
}

void PhysicsManagerJolt::insertConstraint(JPH::Constraint* constraint)
{
	m_physicsSystem->AddConstraint(constraint);
}

void PhysicsManagerJolt::removeConstraint(JPH::Constraint* constraint)
{
	m_physicsSystem->RemoveConstraint(constraint);
}

}
