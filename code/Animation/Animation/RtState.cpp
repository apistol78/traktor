/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/RtState.h"

#include "Animation/Animation/Animation.h"
#include "Animation/Animation/StateContext.h"
#include "Animation/IPoseController.h"
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"

#include <limits>

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.RtState", RtState, Object)

bool RtState::prepare(StateContext& outContext) const
{
	if (m_animation)
	{
		const uint32_t poseCount = m_animation->getKeyPoseCount();
		if (poseCount < 1)
			return false;

		const float duration = m_animation->getKeyPose(poseCount - 1).at;

		outContext.setTime(0.0f);
		outContext.setDuration(duration);
	}
	else if (m_poseController)
	{
		outContext.setTime(0.0f);
		outContext.setDuration(std::numeric_limits< float >::max());
	}
	else
		return false;

	return true;
}

void RtState::evaluate(
	const StateContext& context,
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	Pose& outPose) const
{
	if (m_animation)
		m_animation->getPose(context.getTime(), outPose);
	if (m_poseController)
	{
		AlignedVector< Transform > poseTransforms;
		m_poseController->evaluate(
			context.getTime(),
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
}

}
