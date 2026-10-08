/*
 * TRAKTOR
 * Copyright (c) 2023-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/TimeFromLocomotion.h"

#include "Animation/Animation/Animation.h"
#include "Core/Log/Log.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.TimeFromLocomotion", TimeFromLocomotion, ITransformTime)

void TimeFromLocomotion::calculateTime(const Animation* animation, const Transform& worldTransform, float& inoutTime, float& outDeltaTime)
{
	const uint32_t poseCount = animation->getKeyPoseCount();
	if (poseCount < 2 || !animation->haveRootMotion())
		return;

	const float start = animation->getKeyPose(0).at;
	const float end = animation->getLastKeyPose().at;
	const Vector4 travel = animation->getRootMotion(end).xyz0();
	const float travelLength = travel.length();
	if (end <= start || travelLength < FUZZY_EPSILON)
		return;

	// Distance owner has moved along clip's direction of travel.
	const Vector4 axis = travel / Scalar(travelLength);
	const float distance = std::abs(dot3(worldTransform.rotation() * axis, worldTransform.translation() - m_transform.translation()));
	m_transform = worldTransform;

	// Advance time until clip has travelled as far, wrapping at end of clip.
	float time = clamp(m_time, start, end);
	float remaining = std::fmod(distance, travelLength);
	uint32_t next = 1;
	while (next < poseCount - 1 && animation->getKeyPose(next).at <= time)
		++next;
	for (uint32_t steps = 0; steps < 2 * poseCount && remaining > 0.0f; ++steps)
	{
		const float nextTime = animation->getKeyPose(next).at;
		const float step = dot3(animation->getRootMotion(nextTime) - animation->getRootMotion(time), axis);
		if (step > remaining)
		{
			time += (nextTime - time) * remaining / step;
			break;
		}
		remaining -= std::max(step, 0.0f);
		time = nextTime;
		if (++next >= poseCount)
		{
			time = start;
			next = 1;
		}
	}

	outDeltaTime = (time >= m_time) ? time - m_time : time - m_time + (end - start);
	inoutTime = m_time;
	m_time = time;
}

}
