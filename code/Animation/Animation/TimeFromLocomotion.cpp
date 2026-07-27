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
	Vector4 locomotionDirection = worldTransform * animation->getTotalLocomotion().xyz0();
	Scalar locomotionDistance = locomotionDirection.length();
	if (locomotionDistance < FUZZY_EPSILON)
		return;
	locomotionDirection /= locomotionDistance;
	
	const float distance = dot3(locomotionDirection, (worldTransform.translation() - m_transform.translation()) * Vector4(1.0f, 0.0f, 1.0f));

	outDeltaTime = std::abs(animation->getTimePerDistance() * distance);
    inoutTime = m_time;

	m_transform = worldTransform;
	m_time += outDeltaTime;

	// Ensure time is always positive.
	const float duration = animation->getLastKeyPose().at - animation->getKeyPose(0).at;
	if (duration > 0.0f)
	{
		while (m_time < 0.0f)
			m_time += duration;
		m_time = std::fmod(m_time, duration);
	}
}

}
