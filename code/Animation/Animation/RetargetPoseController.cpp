/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/Animation/RetargetPoseController.h"
#include "Core/Misc/SafeDestroy.h"

namespace traktor::animation
{
	namespace
	{

// Calculate delta transforms from object space pose transforms.
void calculateDeltaTransforms(
	const Skeleton* skeleton,
	const AlignedVector< Transform >& poseTransforms,
	AlignedVector< Transform >& outDeltaTransforms
)
{
	outDeltaTransforms.resize(skeleton->getJointCount(), Transform::identity());
	for (uint32_t i = 0; i < skeleton->getJointCount(); ++i)
	{
		outDeltaTransforms[i] = poseTransforms[i];
		const int32_t parentIndex = skeleton->getJoint(i)->getParent();
		if (parentIndex >= 0)
			outDeltaTransforms[i] = poseTransforms[parentIndex].inverse() * poseTransforms[i];
	}
}

// Concatenate all local pose transforms into object space pose transforms.
void calculatePoseTransforms(
	const Skeleton* skeleton,
	const AlignedVector< Transform >& localPoseTransforms,
	AlignedVector< Transform >& outPoseTransforms
)
{
	outPoseTransforms.resize(skeleton->getJointCount());
	for (uint32_t i = 0; i < skeleton->getJointCount(); ++i)
	{
		outPoseTransforms[i] = localPoseTransforms[i];
		for (int32_t parentIndex = skeleton->getJoint(i)->getParent(); parentIndex >= 0; parentIndex = skeleton->getJoint(parentIndex)->getParent())
			outPoseTransforms[i] = localPoseTransforms[parentIndex] * outPoseTransforms[i];
	}
}


	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.RetargetPoseController", RetargetPoseController, IPoseController)

RetargetPoseController::RetargetPoseController(const resource::Proxy< Skeleton >& animationSkeleton, IPoseController* poseController)
:	m_animationSkeleton(animationSkeleton)
,	m_poseController(poseController)
{
	if (m_animationSkeleton)
	{
		calculateJointTransforms(
			m_animationSkeleton,
			m_jointTransforms
		);
		m_poseTransforms.reserve(m_jointTransforms.size());
	}
}

void RetargetPoseController::destroy()
{
	safeDestroy(m_poseController);
}

void RetargetPoseController::setTransform(const Transform& transform)
{
}

bool RetargetPoseController::evaluate(
	float time,
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	AlignedVector< Transform >& outPoseTransforms
)
{
	if (!m_animationSkeleton || !m_poseController)
		return false;

	const bool result = m_poseController->evaluate(time, deltaTime, worldTransform, m_animationSkeleton, m_jointTransforms, m_poseTransforms);

	// Remap transforms.
	m_remappedPoseTransforms.resize(skeleton->getJointCount(), Transform::identity());
	for (uint32_t i = 0; i < skeleton->getJointCount(); ++i)
	{
		const std::wstring& name = skeleton->getJoint(i)->getName();

		uint32_t sourceJointIndex;
		if (!m_animationSkeleton->findJoint(render::getParameterHandle(name), sourceJointIndex))
			continue;

		m_remappedPoseTransforms[i] = m_poseTransforms[sourceJointIndex];
	}

	// Calculate delta transforms.
	calculateDeltaTransforms(skeleton, m_remappedPoseTransforms, m_deltaPoseTransforms);

	// Adjust delta transforms to not offset translation.
	for (uint32_t i = 0; i < skeleton->getJointCount(); ++i)
	{
		auto joint = skeleton->getJoint(i);
		if (joint->getParent() >= 0)
		{
			const Scalar l1 = (jointTransforms[i].translation() - jointTransforms[joint->getParent()].translation()).length();
			const Scalar l2 = m_deltaPoseTransforms[i].translation().length();
			m_deltaPoseTransforms[i] = Transform(
				m_deltaPoseTransforms[i].translation() * (l1 / l2),
				m_deltaPoseTransforms[i].rotation()
			);
		}
	}

	// Combine all delta transforms into final pose transforms.
	calculatePoseTransforms(skeleton, m_deltaPoseTransforms, outPoseTransforms);
	return result;
}

void RetargetPoseController::estimateVelocities(
	const Skeleton* skeleton,
	AlignedVector< Velocity >& outVelocities
)
{
}

}
