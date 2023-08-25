/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Animation/SkeletonUtils.h"
#include "Animation/Animation/Animation.h"
#include "Animation/Animation/ITransformTime.h"
#include "Animation/Animation/SimpleAnimationController.h"
#include "Core/Math/Const.h"
#include "Core/Math/Random.h"

namespace traktor::animation
{
	namespace
	{

Random s_random;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.SimpleAnimationController", SimpleAnimationController, IPoseController)

SimpleAnimationController::SimpleAnimationController(
	const resource::Proxy< Animation >& animation,
	ITransformTime* transformTime
)
:	m_animation(animation)
,	m_transformTime(transformTime)
,	m_lastTime(std::numeric_limits< float >::max())
{
	m_timeOffset = s_random.nextFloat() * m_animation->getLastKeyPose().at;
}

void SimpleAnimationController::destroy()
{
}

void SimpleAnimationController::setTransform(const Transform& transform)
{
}

bool SimpleAnimationController::evaluate(
	float time,
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	AlignedVector< Transform >& outPoseTransforms
)
{
	if (!m_animation)
		return false;

	if (m_transformTime)
		m_transformTime->calculateTime(m_animation, worldTransform, time, deltaTime);

	// Calculate pose from animation.
	const float poseTime = std::fmod(m_timeOffset + time, m_animation->getLastKeyPose().at);

	m_animation->getPose(poseTime, m_evaluationPose);
	calculatePoseTransforms(
		skeleton,
		&m_evaluationPose,
		outPoseTransforms
	);

	const bool wrapped = (bool)(poseTime < m_lastTime + FUZZY_EPSILON);
	m_lastTime = poseTime;

	// Assume animation is not continous when wrapping.
	return !wrapped;
}

void SimpleAnimationController::estimateVelocities(
	const Skeleton* skeleton,
	AlignedVector< Velocity >& outVelocities
)
{
}

}
