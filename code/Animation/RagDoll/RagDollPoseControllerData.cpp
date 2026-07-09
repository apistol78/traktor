/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/RagDoll/RagDollPoseController.h"
#include "Animation/RagDoll/RagDollPoseControllerData.h"
#include "Animation/RagDoll/RagDollSkeleton.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Physics/Body.h"
#include "Physics/DynamicBodyDesc.h"
#include "Physics/Joint.h"
#include "Physics/JointDesc.h"
#include "Physics/PhysicsManager.h"
#include "Physics/ShapeDesc.h"
#include "Render/Types.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Resource/Proxy.h"

namespace traktor::animation
{
	namespace
	{

uint32_t s_clusterId = 10000;

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.RagDollPoseControllerData", 7, RagDollPoseControllerData, IPoseControllerData)

Ref< IPoseController > RagDollPoseControllerData::createInstance(
	resource::IResourceManager* resourceManager,
	physics::PhysicsManager* physicsManager,
	const Skeleton* skeleton,
	const Transform& worldTransform
) const
{
	if (!physicsManager || !skeleton)
		return nullptr;

	resource::Proxy< RagDollSkeleton > rds;
	if (!resourceManager->bind(m_skeleton, rds))
		return nullptr;

	const AlignedVector< RagDollSkeleton::Limb >& limbs = rds->getLimbs();
	const AlignedVector< RagDollSkeleton::Joint >& joints = rds->getJoints();

	// Object space bind transforms of the animation skeleton; used to relate
	// rag doll limbs to animation joints.
	AlignedVector< Transform > jointTransforms;
	calculateJointTransforms(skeleton, jointTransforms);

	// Unique cluster id so limbs of this rag doll never collide with each other.
	const uint32_t clusterId = s_clusterId++;

	// Create a dynamic body for each rag doll limb.
	// Collision group and mask are left at whatever the limb's shape defines
	// (defaulted for now).
	RefArray< physics::Body > limbBodies;
	RefArray< const physics::ShapeDesc > limbShapes;
	limbBodies.resize(limbs.size());
	limbShapes.resize(limbs.size());
	for (uint32_t i = 0; i < limbs.size(); ++i)
	{
		const RagDollSkeleton::Limb& limb = limbs[i];
		if (!limb.body)
			return nullptr;

		Ref< physics::Body > body = physicsManager->createBody(resourceManager, limb.body, T_FILE_LINE_W);
		if (!body)
			return nullptr;

		body->setTransform(worldTransform * limb.transform);
		body->setClusterId(clusterId);
		body->setEnable(true);

		limbBodies[i] = body;
		limbShapes[i] = limb.body->getShape();
	}

	// Create constraint joints between limbs.
	RefArray< physics::Joint > limbJoints;
	limbJoints.reserve(joints.size());
	for (const RagDollSkeleton::Joint& joint : joints)
	{
		if (!joint.joint || joint.body1 < 0 || joint.body1 >= (int32_t)limbBodies.size())
			continue;

		physics::Body* body1 = limbBodies[joint.body1];
		physics::Body* body2 = (joint.body2 >= 0 && joint.body2 < (int32_t)limbBodies.size()) ? limbBodies[joint.body2].ptr() : nullptr;

		Ref< physics::Joint > limbJoint = physicsManager->createJoint(joint.joint, worldTransform, body1, body2);
		if (!limbJoint)
			continue;

		limbJoints.push_back(limbJoint);
	}

	// Map animation skeleton joints to rag doll limbs based on limb influences, so
	// that animation joint transforms can be reconstructed from the limb bodies
	// during simulation.
	AlignedVector< AlignedVector< RagDollPoseController::Binding > > jointBindings;
	jointBindings.resize(skeleton->getJointCount());
	for (uint32_t i = 0; i < limbs.size(); ++i)
	{
		const RagDollSkeleton::Limb& limb = limbs[i];
		for (const RagDollSkeleton::Influence& influence : limb.influences)
		{
			uint32_t jointIndex;
			if (!skeleton->findJoint(render::getParameterHandle(influence.joint), jointIndex))
			{
				log::warning << L"Rag doll influence references unknown joint \"" << influence.joint << L"\"." << Endl;
				continue;
			}

			RagDollPoseController::Binding binding;
			binding.limb = (int32_t)i;
			binding.weight = Scalar(influence.weight);
			// Fixed offset from the limb (in object space) to the joint (in object space)
			// at bind pose.
			binding.limbToJoint = limb.transform.inverse() * jointTransforms[jointIndex];

			jointBindings[jointIndex].push_back(binding);
		}
	}

	Ref< RagDollPoseController > poseController = new RagDollPoseController();
	if (!poseController->create(limbBodies, limbShapes, limbJoints, jointBindings, worldTransform))
		return nullptr;

	return poseController;
}

void RagDollPoseControllerData::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion< RagDollPoseControllerData >() >= 7);

	s >> resource::Member< RagDollSkeleton >(L"skeleton", m_skeleton);
}

}
