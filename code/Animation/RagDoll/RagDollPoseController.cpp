/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/RagDoll/RagDollPoseController.h"

#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Core/Misc/SafeDestroy.h"
#include "Physics/Body.h"
#include "Physics/Joint.h"

#include <functional>

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.RagDollPoseController", RagDollPoseController, IPoseController)

RagDollPoseController::~RagDollPoseController()
{
	destroy();
}

bool RagDollPoseController::create(
	const RefArray< physics::Body >& limbs,
	const RefArray< const physics::ShapeDesc >& limbShapes,
	const RefArray< physics::Joint >& joints,
	const AlignedVector< AlignedVector< Binding > >& jointBindings,
	const Transform& worldTransform
)
{
	m_limbs = limbs;
	m_limbShapes = limbShapes;
	m_joints = joints;
	m_jointBindings = jointBindings;
	m_worldTransform = worldTransform;
	return true;
}

void RagDollPoseController::destroy()
{
	safeDestroy(m_joints);
	safeDestroy(m_limbs);
	m_limbShapes.resize(0);
	m_jointBindings.clear();
}

void RagDollPoseController::reset(
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& poseTransforms)
{
	m_worldTransform = worldTransform;

	// Place each limb so that, when evaluated, it reproduces the given pose; i.e. invert
	// evaluate()'s "jointObject = worldInv * limbWorld * limbToJoint". Also clear motion so
	// the rag doll starts from rest in the target pose.
	for (uint32_t i = 0; i < m_jointBindings.size() && i < poseTransforms.size(); ++i)
	{
		for (const auto& binding : m_jointBindings[i])
		{
			physics::Body* limb = m_limbs[binding.limb];
			if (!limb)
				continue;

			limb->setTransform(worldTransform * poseTransforms[i] * binding.limbToJoint.inverse());
			limb->setLinearVelocity(Vector4::zero());
			limb->setAngularVelocity(Vector4::zero());
			limb->setActive(true);
		}
	}
}

void RagDollPoseController::setTransform(const Transform& transform)
{
	// Calculate delta transform since last update and move all limbs accordingly.
	const Transform deltaTransform = transform * m_worldTransform.inverse();
	for (auto limb : m_limbs)
	{
		if (limb)
			limb->setTransform(deltaTransform * limb->getTransform());
	}
	m_worldTransform = transform;
}

bool RagDollPoseController::evaluate(
	float time,
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& boneTransforms,
	AlignedVector< Transform >& outPoseTransforms
)
{
	const Transform worldTransformInv = worldTransform.inverse();
	const uint32_t jointCount = skeleton->getJointCount();

	outPoseTransforms.resize(jointCount);

	// Guards against resolving a joint more than once; also breaks any accidental
	// parent cycle in the skeleton.
	AlignedVector< uint8_t > resolved(jointCount, 0);

	// Reconstruct the object space transform of a joint. Joints influenced by one or
	// more limbs are driven directly by their physics bodies; joints without any
	// influence follow their parent, keeping their relative animation pose, so that
	// they are not left behind in object space when the parent is driven by physics.
	std::function< const Transform& (int32_t) > resolve = [&](int32_t index) -> const Transform&
	{
		if (resolved[index])
			return outPoseTransforms[index];
		resolved[index] = 1;

		// Driven directly by any influencing limb bodies.
		if ((size_t)index < m_jointBindings.size())
		{
			Transform pose;
			Scalar totalWeight = 0.0_simd;
			bool first = true;
			for (const auto& binding : m_jointBindings[index])
			{
				const physics::Body* limb = m_limbs[binding.limb];
				if (!limb)
					continue;

				const Transform limbTransform = worldTransformInv * limb->getTransform();
				const Transform jointTransform = limbTransform * binding.limbToJoint;

				if (first)
				{
					pose = jointTransform;
					totalWeight = binding.weight;
					first = false;
				}
				else
				{
					totalWeight += binding.weight;
					pose = lerp(pose, jointTransform, binding.weight / totalWeight);
				}
			}
			if (!first)
			{
				outPoseTransforms[index] = pose;
				return outPoseTransforms[index];
			}
		}

		// Not influenced; follow parent keeping the relative animation pose.
		const int32_t parent = skeleton->getJoint(index)->getParent();
		if (parent >= 0)
		{
			const Transform localTransform = boneTransforms[parent].inverse() * boneTransforms[index];
			outPoseTransforms[index] = resolve(parent) * localTransform;
		}
		else
			outPoseTransforms[index] = boneTransforms[index];

		return outPoseTransforms[index];
	};

	for (uint32_t i = 0; i < jointCount; ++i)
		resolve(i);

	m_worldTransform = worldTransform;
	return true;
}

}
