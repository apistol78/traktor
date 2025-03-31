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

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.RtState", RtState, Object)

bool RtState::prepare(StateContext& outContext) const
{
	if (!m_animation)
		return false;

	const uint32_t poseCount = m_animation->getKeyPoseCount();
	if (poseCount < 1)
		return false;

	const float duration = m_animation->getKeyPose(poseCount - 1).at;

	outContext.setTime(0.0f);
	outContext.setDuration(duration);

	return true;
}

void RtState::evaluate(const StateContext& context, Pose& outPose) const
{
	if (m_animation)
		m_animation->getPose(context.getTime(), outPose);
}

}
