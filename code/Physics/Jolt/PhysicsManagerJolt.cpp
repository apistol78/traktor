/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <thread>
#include "Core/Log/Log.h"
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
#include "Physics/HingeJointDesc.h"
#include "Physics/Hinge2JointDesc.h"
#include "Physics/Mesh.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/SphereShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/Jolt/Conversion.h"
#include "Physics/Jolt/BodyJolt.h"
#include "Physics/Jolt/PhysicsManagerJolt.h"
#include "Resource/IResourceManager.h"

// Keep Jolt includes here, Jolt.h must be first.
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

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

/// Class that determines if two object layers can collide
class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
{
public:
	virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
	{
		switch (inObject1)
		{
		case Layers::NON_MOVING:
			return inObject2 == Layers::MOVING; // Non moving only collides with moving
		case Layers::MOVING:
			return true; // Moving collides with everything
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

// BroadPhaseLayerInterface implementation
// This defines a mapping between object and broadphase layers.
class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
	BPLayerInterfaceImpl()
	{
		// Create a mapping table from object to broad phase layer
		mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
		mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
	}

	virtual JPH::uint GetNumBroadPhaseLayers() const override
	{
		return BroadPhaseLayers::NUM_LAYERS;
	}

	virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
	{
		JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
		return mObjectToBroadPhase[inLayer];
	}

private:
	JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
};

class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
	virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
	{
		switch (inLayer1)
		{
		case Layers::NON_MOVING:
			return inLayer2 == BroadPhaseLayers::MOVING;
		case Layers::MOVING:
			return true;
		default:
			return false;
		}
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
	JPH::RegisterDefaultAllocator();

	JPH::Factory::sInstance = new JPH::Factory();

	JPH::RegisterTypes();

	m_tempAllocator.reset(new JPH::TempAllocatorImpl(10 * 1024 * 1024));
	m_jobSystem.reset(new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1));

	const JPH::uint cMaxBodies = 1024;
	const JPH::uint cNumBodyMutexes = 0;
	const JPH::uint cMaxBodyPairs = 1024;
	const JPH::uint cMaxContactConstraints = 1024;

	BPLayerInterfaceImpl broadPhaseLayerInterface;
	ObjectVsBroadPhaseLayerFilterImpl objectVsBroadPhaseLayerFilter;
	ObjectLayerPairFilterImpl objectVsObjectLayerFilter;

	m_physicsSystem.reset(new JPH::PhysicsSystem);
	m_physicsSystem->Init(
		cMaxBodies,
		cNumBodyMutexes,
		cMaxBodyPairs,
		cMaxContactConstraints,
		broadPhaseLayerInterface,
		objectVsBroadPhaseLayerFilter,
		objectVsObjectLayerFilter
	);

	return true;
}

void PhysicsManagerJolt::destroy()
{
	m_physicsSystem.release();
}

void PhysicsManagerJolt::setGravity(const Vector4& gravity)
{
}

Vector4 PhysicsManagerJolt::getGravity() const
{
	return Vector4::zero();
}

Ref< Body > PhysicsManagerJolt::createBody(resource::IResourceManager* resourceManager, const BodyDesc* desc, const wchar_t* const tag)
{
	return nullptr;
}

Ref< Body > PhysicsManagerJolt::createBody(resource::IResourceManager* resourceManager, const BodyDesc* desc, const Mesh* mesh, const wchar_t* const tag)
{
	JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
	JPH::Body* body = nullptr;

	if (auto staticDesc = dynamic_type_cast< const StaticBodyDesc* >(desc))
	{
		JPH::VertexList vertexList;	// Array<Float3>
		for (const auto& vertex : mesh->getVertices())
		{
			vertexList.push_back(JPH::Float3(
				vertex.x(), vertex.y(), vertex.z()
			));
		}

		JPH::IndexedTriangleList triangleList;	// Array<IndexedTriangle>
		for (const auto& triangle : mesh->getHullTriangles())
		{
			triangleList.push_back(JPH::IndexedTriangle(
				triangle.indices[0],
				triangle.indices[1],
				triangle.indices[2],
				triangle.material
			));
		}

		// // Next we can create a rigid body to serve as the floor, we make a large box
		// // Create the settings for the collision volume (the shape).
		// // Note that for simple shapes (like boxes) you can also directly construct a BoxShape.
		JPH::MeshShapeSettings shapeSettings(vertexList, triangleList);
		shapeSettings.SetEmbedded(); // A ref counted object on the stack (base class RefTarget) should be marked as such to prevent it from being freed when its reference count goes to 0.

		// Create the shape
		JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
		JPH::ShapeRefC shape = shapeResult.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()

		// Create the settings for the body itself. Note that here you can also set other properties like the restitution / friction.
		JPH::BodyCreationSettings settings(
			shape,
			JPH::RVec3(0.0_r, 0.0_r, 0.0_r),
			JPH::Quat::sIdentity(),
			staticDesc->isKinematic() ? JPH::EMotionType::Kinematic : JPH::EMotionType::Static,
			Layers::NON_MOVING
		);

		// Create the actual rigid body
		body = bodyInterface.CreateBody(settings);
		if (!body)
			return nullptr;

		// Add it to the world
		bodyInterface.AddBody(body->GetID(), JPH::EActivation::Activate);
	}
	else if (auto dynamicDesc = dynamic_type_cast< const DynamicBodyDesc* >(desc))
	{
		JPH::VertexList vertexList;	// Array<Float3>
		for (const auto& vertex : mesh->getVertices())
		{
			vertexList.push_back(JPH::Float3(
				vertex.x(), vertex.y(), vertex.z()
			));
		}

		JPH::IndexedTriangleList triangleList;	// Array<IndexedTriangle>
		for (const auto& triangle : mesh->getShapeTriangles())
		{
			triangleList.push_back(JPH::IndexedTriangle(
				triangle.indices[0],
				triangle.indices[1],
				triangle.indices[2],
				triangle.material
			));
		}

		// // Next we can create a rigid body to serve as the floor, we make a large box
		// // Create the settings for the collision volume (the shape).
		// // Note that for simple shapes (like boxes) you can also directly construct a BoxShape.
		JPH::MeshShapeSettings shapeSettings(vertexList, triangleList);
		shapeSettings.SetEmbedded(); // A ref counted object on the stack (base class RefTarget) should be marked as such to prevent it from being freed when its reference count goes to 0.

		// Create the shape
		JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
		JPH::ShapeRefC shape = shapeResult.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()

		// Create the settings for the body itself. Note that here you can also set other properties like the restitution / friction.
		JPH::BodyCreationSettings settings(
			shape,
			JPH::RVec3(0.0_r, 0.0_r, 0.0_r),
			JPH::Quat::sIdentity(),
			JPH::EMotionType::Dynamic,
			Layers::MOVING
		);

		// Create the actual rigid body
		body = bodyInterface.CreateBody(settings);
		if (!body)
			return nullptr;

		// Add it to the world
		bodyInterface.AddBody(body->GetID(), JPH::EActivation::Activate);
	}
	T_FATAL_ASSERT(body != nullptr);
	return new BodyJolt(tag, body);
}

Ref< Joint > PhysicsManagerJolt::createJoint(const JointDesc* desc, const Transform& transform, Body* body1, Body* body2)
{
	return nullptr;
}

void PhysicsManagerJolt::update(float simulationDeltaTime, bool issueCollisionEvents)
{
	const int cCollisionSteps = 1;
	m_physicsSystem->Update(simulationDeltaTime, cCollisionSteps, m_tempAllocator.ptr(), m_jobSystem.ptr());
}

void PhysicsManagerJolt::solveConstraints(const RefArray< Body >& bodies, const RefArray< Joint >& joints)
{
}

RefArray< Body > PhysicsManagerJolt::getBodies() const
{
	return RefArray< Body >();
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
	QueryResult& outResult
) const
{
	return false;
}

bool PhysicsManagerJolt::queryShadowRay(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	const QueryFilter& queryFilter,
	uint32_t queryTypes
) const
{
	return false;
}

uint32_t PhysicsManagerJolt::querySphere(
	const Vector4& at,
	float radius,
	const QueryFilter& queryFilter,
	uint32_t queryTypes,
	RefArray< Body >& outBodies
) const
{
	return false;
}

bool PhysicsManagerJolt::querySweep(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	float radius,
	const QueryFilter& queryFilter,
	QueryResult& outResult
) const
{
	return false;
}

bool PhysicsManagerJolt::querySweep(
	const Body* body,
	const Quaternion& orientation,
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	const QueryFilter& queryFilter,
	QueryResult& outResult
) const
{
	return false;
}

void PhysicsManagerJolt::querySweep(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	float radius,
	const QueryFilter& queryFilter,
	AlignedVector< QueryResult >& outResult
) const
{
}

void PhysicsManagerJolt::queryOverlap(
	const Body* body,
	RefArray< Body >& outResult
) const
{
}

void PhysicsManagerJolt::queryTriangles(const Vector4& center, float radius, AlignedVector< TriangleResult >& outTriangles) const
{
}

void PhysicsManagerJolt::getStatistics(PhysicsStatistics& outStatistics) const
{
	outStatistics.bodyCount = 0;
	outStatistics.activeCount = 0;
	outStatistics.manifoldCount = 0;
	outStatistics.queryCount = 0;
}

}
