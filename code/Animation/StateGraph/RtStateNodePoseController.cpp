/*
 * TRAKTOR
 * Copyright (c) 2025-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/StateGraph/RtStateNodePoseController.h"

#include "Animation/IPoseController.h"
#include "Animation/ITransformTime.h"
#include "Animation/Joint.h"
#include "Animation/Pose.h"
#include "Animation/Skeleton.h"
#include "Animation/StateGraph/RtStateContext.h"
#include "Core/Misc/SafeDestroy.h"

#include <limits>

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.RtStateNodePoseController", RtStateNodePoseController, RtStateNode)

void RtStateNodePoseController::destroy()
{
	safeDestroy(m_poseController);
	m_transformTime = nullptr;
}

void RtStateNodePoseController::reset(
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& poseTransforms)
{
	if (m_poseController)
		m_poseController->reset(worldTransform, skeleton, poseTransforms);
}

float RtStateNodePoseController::evaluate(
	const RtStateContext& /*context*/,
	float time,
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	Pose& outPose,
	Vector4* outRootMotion)
{
	if (outRootMotion)
		*outRootMotion = Vector4::zero();

	if (m_poseController)
	{
		AlignedVector< Transform > poseTransforms;
		m_poseController->evaluate(
			time,
			deltaTime,
			worldTransform,
			skeleton,
			jointTransforms,
			poseTransforms);

		// Convert absolute transforms into a Pose instance.
		for (int32_t i = 0; i < skeleton->getJointCount(); ++i)
		{
			const Joint* joint = skeleton->getJoint(i);

			Transform deltaTransform = poseTransforms[i];
			if (joint->getParent() >= 0)
				deltaTransform = poseTransforms[joint->getParent()].inverse() * poseTransforms[i];

			outPose.setJointTransform(i, deltaTransform);
		}
	}

	return std::numeric_limits< float >::max();
}

IPoseController* RtStateNodePoseController::getActivePoseController() const
{
	return m_poseController ? m_poseController->getActivePoseController() : nullptr;
}

void RtStateNodePoseController::getPoseControllersOf(const TypeInfo& type, RefArray< IPoseController >& outControllers) const
{
	if (m_poseController != nullptr)
		m_poseController->getPoseControllersOf(type, outControllers);
}

}
