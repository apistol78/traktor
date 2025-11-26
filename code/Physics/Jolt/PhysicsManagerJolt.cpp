/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/Jolt/PhysicsManagerJolt.h"

#include "Core/Log/Log.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Const.h"
#include "Core/Math/Format.h"
#include "Core/Misc/Save.h"
#include "Core/Thread/Acquire.h"
#include "Heightfield/Heightfield.h"
#include "Physics/AxisJointDesc.h"
#include "Physics/BallJointDesc.h"
#include "Physics/BoxShapeDesc.h"
#include "Physics/CapsuleShapeDesc.h"
#include "Physics/CollisionListener.h"
#include "Physics/CollisionSpecification.h"
#include "Physics/CompoundShapeDesc.h"
#include "Physics/ConeTwistJointDesc.h"
#include "Physics/CylinderShapeDesc.h"
#include "Physics/DofJointDesc.h"
#include "Physics/DynamicBodyDesc.h"
#include "Physics/HeightfieldShapeDesc.h"
#include "Physics/Hinge2JointDesc.h"
#include "Physics/HingeJointDesc.h"
#include "Physics/Jolt/BodyJolt.h"
#include "Physics/Jolt/Conversion.h"
#include "Physics/Mesh.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/SphereShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Resource/IResourceManager.h"

#include <algorithm>
#include <cstring>
#include <thread>

// Keep Jolt includes here, Jolt.h must be first.
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/PhysicsMaterialSimple.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
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
static constexpr JPH::ObjectLayer NON_MOVING = 0;
static constexpr JPH::ObjectLayer MOVING = 1;
static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
};

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

namespace BroadPhaseLayers
{
static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
static constexpr JPH::BroadPhaseLayer MOVING(1);
static constexpr JPH::uint NUM_LAYERS(2);
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

class MyContactListener : public JPH::ContactListener
{
public:
	virtual JPH::ValidateResult OnContactValidate(const JPH::Body& body1, const JPH::Body& body2, JPH::RVec3Arg baseOffset, const JPH::CollideShapeResult& collisionResult) override
	{
		return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
	}

	virtual void OnContactAdded(const JPH::Body& body1, const JPH::Body& body2, const JPH::ContactManifold& manifold, JPH::ContactSettings& inoutSettings) override
	{
	}

	virtual void OnContactPersisted(const JPH::Body& body1, const JPH::Body& body2, const JPH::ContactManifold& manifold, JPH::ContactSettings& inoutSettings) override
	{
	}

	virtual void OnContactRemoved(const JPH::SubShapeIDPair& subShapePair) override
	{
	}
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

	m_tempAllocator.reset(new JPH::TempAllocatorImpl(10 * 1024 * 1024));
	m_jobSystem.reset(new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1));

	const JPH::uint cMaxBodies = 1024;
	const JPH::uint cNumBodyMutexes = 0;
	const JPH::uint cMaxBodyPairs = 1024;
	const JPH::uint cMaxContactConstraints = 1024;

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

	m_contactListener.reset(new MyContactListener());
	m_physicsSystem->SetContactListener(m_contactListener.ptr());

	m_physicsSystem->SetGravity(JPH::Vec3(0.0f, -9.2f, 0.0f));

	m_timeScale = desc.timeScale;
	return true;
}

void PhysicsManagerJolt::destroy()
{
	while (!m_bodies.empty())
	{
		Ref< Body > body = m_bodies.front();
		T_FATAL_ASSERT(body != nullptr);
		body->destroy();
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
	JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
	JPH::Body* body = nullptr;

	if (!desc)
		return nullptr;

	const ShapeDesc* shapeDesc = desc->getShape();
	if (!shapeDesc)
	{
		log::error << L"Unable to create body, no shape defined." << Endl;
		return nullptr;
	}

	// Resolve collision group and mask value.
	uint32_t mergedCollisionGroup = 0;
	for (const auto& group : desc->getShape()->getCollisionGroup())
	{
		resource::Proxy< CollisionSpecification > collisionGroup;
		if (!resourceManager->bind(group, collisionGroup))
		{
			log::error << L"Unable to bind collision group specification." << Endl;
			return nullptr;
		}
		mergedCollisionGroup |= collisionGroup->getBitMask();
	}

	uint32_t mergedCollisionMask = 0;
	for (const auto& mask : desc->getShape()->getCollisionMask())
	{
		resource::Proxy< CollisionSpecification > collisionMask;
		if (!resourceManager->bind(mask, collisionMask))
		{
			log::error << L"Unable to bind collision mask specification." << Endl;
			return nullptr;
		}
		mergedCollisionMask |= collisionMask->getBitMask();
	}

	// Create collision shape.
	if (const SphereShapeDesc* sphereShape = dynamic_type_cast< const SphereShapeDesc* >(shapeDesc))
	{
		JPH::SphereShapeSettings shapeSettings(sphereShape->getRadius());
		shapeSettings.SetEmbedded();

		JPH::ShapeRefC shape;

		const Transform localTransform = shapeDesc->getLocalTransform();
		if (localTransform != Transform::identity())
		{
			JPH::MutableCompoundShapeSettings compoundSettings;
			compoundSettings.AddShape(
				convertToJolt(localTransform.translation()),
				convertToJolt(localTransform.rotation()),
				&shapeSettings);

			 JPH::ShapeSettings::ShapeResult shapeResult = compoundSettings.Create();
			 shape = shapeResult.Get();
		}
		else
		{
			 JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
			 shape = shapeResult.Get();
		}

		JPH::BodyCreationSettings settings;

		if (auto staticDesc = dynamic_type_cast< const StaticBodyDesc* >(desc))
		{
			settings = JPH::BodyCreationSettings(
				shape,
				JPH::RVec3(0.0_r, 0.0_r, 0.0_r),
				JPH::Quat::sIdentity(),
				JPH::EMotionType::Static,
				Layers::NON_MOVING);
			settings.mFriction = staticDesc->getFriction();
			settings.mRestitution = staticDesc->getRestitution();
		}
		else if (auto dynamicDesc = dynamic_type_cast< const DynamicBodyDesc* >(desc))
		{
			settings = JPH::BodyCreationSettings(
				shape,
				JPH::RVec3(0.0_r, 0.0_r, 0.0_r),
				JPH::Quat::sIdentity(),
				JPH::EMotionType::Dynamic,
				Layers::MOVING);
			settings.mLinearDamping = dynamicDesc->getLinearDamping();
			settings.mAngularDamping = dynamicDesc->getAngularDamping();
			settings.mFriction = dynamicDesc->getFriction();
			settings.mRestitution = dynamicDesc->getRestitution();
		}

		body = bodyInterface.CreateBody(settings);
		if (!body)
			return nullptr;

		Ref< BodyJolt > bj = new BodyJolt(
			tag,
			this,
			m_physicsSystem.ptr(),
			body,
			0.0f,
			Vector4::zero(),
			mergedCollisionGroup,
			mergedCollisionMask);
		m_bodies.push_back(bj);
		return bj;
	}
	else if (const MeshShapeDesc* meshShape = dynamic_type_cast< const MeshShapeDesc* >(shapeDesc))
	{
		resource::Proxy< Mesh > mesh;
		if (!resourceManager->bind(meshShape->getMesh(), mesh))
		{
			log::error << L"Unable to load collision mesh resource " << Guid(meshShape->getMesh()).format() << L"." << Endl;
			return nullptr;
		}

		return createBody(resourceManager, desc, mesh, mergedCollisionGroup, mergedCollisionMask, tag);
	}
	else if (const HeightfieldShapeDesc* heightfieldShape = dynamic_type_cast< const HeightfieldShapeDesc* >(shapeDesc))
	{
		resource::Proxy< hf::Heightfield > heightfield;
		if (!resourceManager->bind(heightfieldShape->getHeightfield(), heightfield))
		{
			log::error << L"Unable to load heightfield resource." << Endl;
			return nullptr;
		}

		AlignedVector< float > samples;
		samples.resize(heightfield->getSize() * heightfield->getSize());

		for (int32_t y = 0; y < heightfield->getSize(); ++y)
			for (int32_t x = 0; x < heightfield->getSize(); ++x)
				samples[x + y * heightfield->getSize()] = heightfield->getGridHeightNearest(x, y);

		const Vector4 s(
			1.0f / heightfield->getSize(),
			1.0f,
			1.0f / heightfield->getSize(),
			1.0f);

		const Vector4& worldExtent = heightfield->getWorldExtent();

		JPH::HeightFieldShapeSettings shapeSettings(
			samples.c_ptr(),
			convertToJolt(-worldExtent * 0.5_simd), // offset
			convertToJolt(worldExtent * s),			// scale
			heightfield->getSize());
		shapeSettings.SetEmbedded();

		JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
		JPH::ShapeRefC shape = shapeResult.Get();

		JPH::BodyCreationSettings settings(
			shape,
			JPH::RVec3(0.0_r, 0.0_r, 0.0_r),
			JPH::Quat::sIdentity(),
			JPH::EMotionType::Static,
			Layers::NON_MOVING);

		if (auto staticDesc = dynamic_type_cast< const StaticBodyDesc* >(desc))
		{
			settings.mFriction = staticDesc->getFriction();
			settings.mRestitution = staticDesc->getRestitution();
		}
		else if (auto dynamicDesc = dynamic_type_cast< const DynamicBodyDesc* >(desc))
		{
			settings.mLinearDamping = dynamicDesc->getLinearDamping();
			settings.mAngularDamping = dynamicDesc->getAngularDamping();
			settings.mFriction = dynamicDesc->getFriction();
			settings.mRestitution = dynamicDesc->getRestitution();
		}

		body = bodyInterface.CreateBody(settings);
		if (!body)
			return nullptr;

		Ref< BodyJolt > bj = new BodyJolt(
			tag,
			this,
			m_physicsSystem.ptr(),
			body,
			0.0f,
			Vector4::zero(),
			mergedCollisionGroup,
			mergedCollisionMask);
		m_bodies.push_back(bj);
		return bj;
	}
	else
	{
		log::error << L"Unsupported shape type \"" << type_name(shapeDesc) << L"\"." << Endl;
	}

	return nullptr;
}

Ref< Body > PhysicsManagerJolt::createBody(resource::IResourceManager* resourceManager, const BodyDesc* desc, const Mesh* mesh, const wchar_t* const tag)
{
	// Resolve collision group and mask value.
	uint32_t mergedCollisionGroup = 0;
	for (const auto& group : desc->getShape()->getCollisionGroup())
	{
		resource::Proxy< CollisionSpecification > collisionGroup;
		if (!resourceManager->bind(group, collisionGroup))
		{
			log::error << L"Unable to bind collision group specification." << Endl;
			return nullptr;
		}
		mergedCollisionGroup |= collisionGroup->getBitMask();
	}

	uint32_t mergedCollisionMask = 0;
	for (const auto& mask : desc->getShape()->getCollisionMask())
	{
		resource::Proxy< CollisionSpecification > collisionMask;
		if (!resourceManager->bind(mask, collisionMask))
		{
			log::error << L"Unable to bind collision mask specification." << Endl;
			return nullptr;
		}
		mergedCollisionMask |= collisionMask->getBitMask();
	}

	return createBody(resourceManager, desc, mesh, mergedCollisionGroup, mergedCollisionMask, tag);
}

Ref< Joint > PhysicsManagerJolt::createJoint(const JointDesc* desc, const Transform& transform, Body* body1, Body* body2)
{
	return nullptr;
}

void PhysicsManagerJolt::update(float simulationDeltaTime, bool issueCollisionEvents)
{
	const int cCollisionSteps = 2;
	m_physicsSystem->Update(simulationDeltaTime * m_timeScale, cCollisionSteps, m_tempAllocator.ptr(), m_jobSystem.ptr());
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
	return 0;
}

bool PhysicsManagerJolt::queryPoint(const Vector4& at, float margin, QueryResult& outResult) const
{
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
	const JPH::NarrowPhaseQuery& narrowPhaseQuery = m_physicsSystem->GetNarrowPhaseQuery();

	class RayCollector : public JPH::CastRayCollector
	{
	public:
		explicit RayCollector(const PhysicsManagerJolt* outer, const JPH::RRayCast& ray, const QueryFilter& queryFilter, QueryResult& outResult)
			: m_outer(outer)
			, m_ray(ray)
			, m_queryFilter(queryFilter)
			, m_outResult(outResult)
		{
		}

		virtual void AddHit(const JPH::RayCastResult& result) override
		{
			if (result.mFraction < GetEarlyOutFraction())
			{
				JPH::BodyLockRead lock(m_outer->m_physicsSystem->GetBodyLockInterface(), result.mBodyID);
				if (lock.Succeeded())
				{
					const JPH::Body& hitBody = lock.GetBody();

					BodyJolt* unwrappedBody = (BodyJolt*)hitBody.GetUserData();
					if (!unwrappedBody)
						return;

					if (m_queryFilter.ignoreClusterId != 0 && unwrappedBody->getClusterId() == m_queryFilter.ignoreClusterId)
						return;

					const uint32_t group = unwrappedBody->getCollisionGroup();
					if ((group & m_queryFilter.includeGroup) == 0 || (group & m_queryFilter.ignoreGroup) != 0)
						return;

					JPH::Vec3 position = m_ray.GetPointOnRay(result.mFraction);
					JPH::Vec3 normal = hitBody.GetWorldSpaceSurfaceNormal(result.mSubShapeID2, position);

					m_outResult.body = unwrappedBody;
					m_outResult.position = convertFromJolt(position, 1.0f);
					m_outResult.normal = convertFromJolt(normal, 0.0f);
					m_outResult.fraction = result.mFraction;
					// m_outResult.material = ;

					m_anyHit = true;

					UpdateEarlyOutFraction(result.mFraction);
				}
			}
		}

		bool AnyHit() const { return m_anyHit; }

	private:
		const PhysicsManagerJolt* m_outer;
		const JPH::RRayCast& m_ray;
		const QueryFilter& m_queryFilter;
		QueryResult& m_outResult;
		bool m_anyHit = false;
	};

	const JPH::RRayCast ray{ convertToJolt(at), convertToJolt(direction * Scalar(maxLength)) };

	JPH::RayCastSettings settings;
	settings.mBackFaceModeTriangles = JPH::EBackFaceMode::IgnoreBackFaces;
	settings.mBackFaceModeConvex = JPH::EBackFaceMode::IgnoreBackFaces;
	settings.mTreatConvexAsSolid = true;

	RayCollector collector(this, ray, queryFilter, outResult);
	narrowPhaseQuery.CastRay(ray, settings, collector);

	outResult.distance = dot3(outResult.position - at, direction);
	return collector.AnyHit();
}

bool PhysicsManagerJolt::queryShadowRay(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	const QueryFilter& queryFilter,
	uint32_t queryTypes) const
{
	return false;
}

uint32_t PhysicsManagerJolt::querySphere(
	const Vector4& at,
	float radius,
	const QueryFilter& queryFilter,
	uint32_t queryTypes,
	RefArray< Body >& outBodies) const
{
	return false;
}

bool PhysicsManagerJolt::querySweep(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	float radius,
	const QueryFilter& queryFilter,
	QueryResult& outResult) const
{
	const JPH::NarrowPhaseQuery& narrowPhaseQuery = m_physicsSystem->GetNarrowPhaseQuery();

	class SweepCollector : public JPH::CastShapeCollector
	{
	public:
		explicit SweepCollector(const PhysicsManagerJolt* outer, const JPH::RShapeCast& shapeCast, const QueryFilter& queryFilter, QueryResult& outResult)
			: m_outer(outer)
			, m_shapeCast(shapeCast)
			, m_queryFilter(queryFilter)
			, m_outResult(outResult)
		{
		}

		virtual void AddHit(const JPH::ShapeCastResult& result) override
		{
			if (result.mFraction < GetEarlyOutFraction())
			{
				JPH::BodyLockRead lock(m_outer->m_physicsSystem->GetBodyLockInterface(), result.mBodyID2);
				if (lock.Succeeded())
				{
					const JPH::Body& hitBody = lock.GetBody();

					BodyJolt* unwrappedBody = (BodyJolt*)hitBody.GetUserData();
					if (!unwrappedBody)
						return;

					if (m_queryFilter.ignoreClusterId != 0 && unwrappedBody->getClusterId() == m_queryFilter.ignoreClusterId)
						return;

					const uint32_t group = unwrappedBody->getCollisionGroup();
					if ((group & m_queryFilter.includeGroup) == 0 || (group & m_queryFilter.ignoreGroup) != 0)
						return;

					JPH::Vec3 position = m_shapeCast.GetPointOnRay(result.mFraction);
					JPH::Vec3 normal = -result.mPenetrationAxis.Normalized();

					m_outResult.body = unwrappedBody;
					m_outResult.position = convertFromJolt(position, 1.0f);
					m_outResult.normal = convertFromJolt(normal, 0.0f);
					m_outResult.fraction = result.mFraction;
					// m_outResult.material = ;

					m_anyHit = true;

					UpdateEarlyOutFraction(result.mFraction);
				}
			}
		}

		bool AnyHit() const { return m_anyHit; }

	private:
		const PhysicsManagerJolt* m_outer;
		const JPH::RShapeCast& m_shapeCast;
		const QueryFilter& m_queryFilter;
		QueryResult& m_outResult;
		bool m_anyHit = false;
	};

	JPH::SphereShape sphere(radius);
	sphere.SetEmbedded();

	JPH::RShapeCast shapeCast(
		&sphere,
		JPH::Vec3::sReplicate(1.0f),
		JPH::RMat44::sTranslation(convertToJolt(at)),
		convertToJolt(direction * Scalar(maxLength)));

	JPH::ShapeCastSettings settings;
	settings.mUseShrunkenShapeAndConvexRadius = true;
	settings.mReturnDeepestPoint = true;

	SweepCollector collector(this, shapeCast, queryFilter, outResult);
	narrowPhaseQuery.CastShape(shapeCast, settings, JPH::Vec3::sReplicate(0.0f), collector);

	outResult.distance = dot3(outResult.position - at, direction);
	return collector.AnyHit();
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
	return false;
}

void PhysicsManagerJolt::querySweep(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	float radius,
	const QueryFilter& queryFilter,
	AlignedVector< QueryResult >& outResult) const
{
	const JPH::NarrowPhaseQuery& narrowPhaseQuery = m_physicsSystem->GetNarrowPhaseQuery();

	class SweepCollector : public JPH::CastShapeCollector
	{
	public:
		explicit SweepCollector(const PhysicsManagerJolt* outer, const JPH::RShapeCast& shapeCast, const QueryFilter& queryFilter, AlignedVector< QueryResult >& outResult)
			: m_outer(outer)
			, m_shapeCast(shapeCast)
			, m_queryFilter(queryFilter)
			, m_outResult(outResult)
		{
		}

		virtual void AddHit(const JPH::ShapeCastResult& result) override
		{
			if (result.mFraction < GetEarlyOutFraction())
			{
				JPH::BodyLockRead lock(m_outer->m_physicsSystem->GetBodyLockInterface(), result.mBodyID2);
				if (lock.Succeeded())
				{
					const JPH::Body& hitBody = lock.GetBody();

					BodyJolt* unwrappedBody = (BodyJolt*)hitBody.GetUserData();
					if (!unwrappedBody)
						return;

					if (m_queryFilter.ignoreClusterId != 0 && unwrappedBody->getClusterId() == m_queryFilter.ignoreClusterId)
						return;

					const uint32_t group = unwrappedBody->getCollisionGroup();
					if ((group & m_queryFilter.includeGroup) == 0 || (group & m_queryFilter.ignoreGroup) != 0)
						return;

					JPH::Vec3 position = m_shapeCast.GetPointOnRay(result.mFraction);
					JPH::Vec3 normal = -result.mPenetrationAxis.Normalized();

					auto& outResult = m_outResult.push_back();
					outResult.body = unwrappedBody;
					outResult.position = convertFromJolt(position, 1.0f);
					outResult.normal = convertFromJolt(normal, 0.0f);
					outResult.fraction = result.mFraction;
					// outResult.material = ;

					UpdateEarlyOutFraction(result.mFraction);
				}
			}
		}

	private:
		const PhysicsManagerJolt* m_outer;
		const JPH::RShapeCast& m_shapeCast;
		const QueryFilter& m_queryFilter;
		AlignedVector< QueryResult >& m_outResult;
	};

	JPH::SphereShape sphere(radius);
	sphere.SetEmbedded();

	JPH::RShapeCast shapeCast(
		&sphere,
		JPH::Vec3::sReplicate(1.0f),
		JPH::RMat44::sTranslation(convertToJolt(at)),
		convertToJolt(direction * Scalar(maxLength)));

	JPH::ShapeCastSettings settings;
	settings.mUseShrunkenShapeAndConvexRadius = true;
	settings.mReturnDeepestPoint = true;

	SweepCollector collector(this, shapeCast, queryFilter, outResult);
	narrowPhaseQuery.CastShape(shapeCast, settings, JPH::Vec3::sReplicate(0.0f), collector);

	for (auto& result : outResult)
		result.distance = dot3(result.position - at, direction);
}

void PhysicsManagerJolt::queryOverlap(
	const Body* body,
	RefArray< Body >& outResult) const
{
}

void PhysicsManagerJolt::queryTriangles(const Vector4& center, float radius, AlignedVector< TriangleResult >& outTriangles) const
{
}

void PhysicsManagerJolt::getStatistics(PhysicsStatistics& outStatistics) const
{
	outStatistics.bodyCount = (uint32_t)m_bodies.size();
	outStatistics.activeCount = 0;
	outStatistics.manifoldCount = 0;
	outStatistics.queryCount = 0;
}

Ref< Body > PhysicsManagerJolt::createBody(resource::IResourceManager* resourceManager, const BodyDesc* desc, const Mesh* mesh, uint32_t collisionGroup, uint32_t collisionMask, const wchar_t* const tag)
{
	JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
	JPH::Body* body = nullptr;

	const Vector4 centerOfGravity = mesh->getOffset();
	float inverseMass = 0.0f;

	const ShapeDesc* shapeDesc = desc->getShape();
	if (!shapeDesc)
	{
		log::error << L"Unable to create body, no shape defined." << Endl;
		return nullptr;
	}

	if (auto staticDesc = dynamic_type_cast< const StaticBodyDesc* >(desc))
	{
		JPH::VertexList vertexList;
		vertexList.reserve(mesh->getVertices().size());
		for (const auto& vertex : mesh->getVertices())
			vertexList.push_back(JPH::Float3(
				vertex.x(),
				vertex.y(),
				vertex.z()));

		JPH::IndexedTriangleList triangleList;
		triangleList.reserve(mesh->getShapeTriangles().size());
		for (const auto& triangle : mesh->getShapeTriangles())
			triangleList.push_back(JPH::IndexedTriangle(
				triangle.indices[2],
				triangle.indices[1],
				triangle.indices[0],
				0 // triangle.material
				));

		JPH::MeshShapeSettings shapeSettings(vertexList, triangleList);
		shapeSettings.SetEmbedded();

		JPH::ShapeRefC shape;

		const Transform localTransform = shapeDesc->getLocalTransform();
		if (localTransform != Transform::identity())
		{
			JPH::MutableCompoundShapeSettings compoundSettings;
			compoundSettings.AddShape(
				convertToJolt(localTransform.translation()),
				convertToJolt(localTransform.rotation()),
				&shapeSettings);

			 JPH::ShapeSettings::ShapeResult shapeResult = compoundSettings.Create();
			 shape = shapeResult.Get();
		}
		else
		{
			 JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
			 shape = shapeResult.Get();
		}

		JPH::BodyCreationSettings settings(
			shape,
			JPH::RVec3(0.0_r, 0.0_r, 0.0_r),
			JPH::Quat::sIdentity(),
			staticDesc->isKinematic() ? JPH::EMotionType::Kinematic : JPH::EMotionType::Static,
			Layers::NON_MOVING);

		body = bodyInterface.CreateBody(settings);
		if (!body)
			return nullptr;
	}
	else if (auto dynamicDesc = dynamic_type_cast< const DynamicBodyDesc* >(desc))
	{
		JPH::Array< JPH::Vec3 > vertexList;
		Aabb3 boundingBox;

		const auto& vertices = mesh->getVertices();
		const auto& hullIndices = mesh->getHullIndices();

		vertexList.reserve(hullIndices.size());
		for (const auto& hullIndex : hullIndices)
		{
			const auto& vertex = vertices[hullIndex];
			vertexList.push_back(JPH::Vec3(
				vertex.x(),
				vertex.y(),
				vertex.z()));
			boundingBox.contain(vertex);
		}

		JPH::ConvexHullShapeSettings shapeSettings(vertexList);
		shapeSettings.SetEmbedded();

		JPH::ShapeRefC shape;

		const Transform localTransform = shapeDesc->getLocalTransform();
		if (localTransform != Transform::identity())
		{
			JPH::MutableCompoundShapeSettings compoundSettings;
			compoundSettings.AddShape(
				convertToJolt(localTransform.translation()),
				convertToJolt(localTransform.rotation()),
				&shapeSettings);

			 JPH::ShapeSettings::ShapeResult shapeResult = compoundSettings.Create();
			 shape = shapeResult.Get();
		}
		else
		{
			 JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
			 shape = shapeResult.Get();
		}

		JPH::BodyCreationSettings settings(
			shape,
			JPH::RVec3(0.0_r, 0.0_r, 0.0_r),
			JPH::Quat::sIdentity(),
			JPH::EMotionType::Dynamic,
			Layers::MOVING);
		settings.mLinearDamping = dynamicDesc->getLinearDamping();
		settings.mAngularDamping = dynamicDesc->getAngularDamping();
		settings.mFriction = dynamicDesc->getFriction();
		settings.mRestitution = dynamicDesc->getRestitution();

		const Vector4 bbs = boundingBox.getExtent() * 2.0_simd;

		const float mass = dynamicDesc->getMass();
		settings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
		settings.mMassPropertiesOverride.SetMassAndInertiaOfSolidBox(convertToJolt(bbs), mass / (bbs.x() * bbs.y() * bbs.z()));

		body = bodyInterface.CreateBody(settings);
		if (!body)
			return nullptr;

		inverseMass = 1.0f / mass;
	}
	T_FATAL_ASSERT(body != nullptr);

	Ref< BodyJolt > bj = new BodyJolt(
		tag,
		this,
		m_physicsSystem.ptr(),
		body,
		inverseMass,
		centerOfGravity,
		collisionGroup,
		collisionMask);
	m_bodies.push_back(bj);
	return bj;
}

void PhysicsManagerJolt::destroyBody(BodyJolt* body)
{
	const bool removed = m_bodies.remove(body);
	T_FATAL_ASSERT(removed);
}

}
