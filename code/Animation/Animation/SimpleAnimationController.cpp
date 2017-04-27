/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/SkeletonUtils.h"
#include "Animation/Animation/Animation.h"
#include "Animation/Animation/SimpleAnimationController.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.SimpleAnimationController", SimpleAnimationController, IPoseController)

SimpleAnimationController::SimpleAnimationController(const resource::Proxy< Animation >& animation, bool linearInterpolation)
:	m_animation(animation)
,	m_linearInterpolation(linearInterpolation)
,	m_time(0.0f)
,	m_indexHint(-1)
{
}

void SimpleAnimationController::destroy()
{
}

void SimpleAnimationController::setTransform(const Transform& transform)
{
}

bool SimpleAnimationController::evaluate(
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	AlignedVector< Transform >& outPoseTransforms,
	bool& outUpdateController
)
{
	if (!m_animation)
		return false;

	Pose pose;
	m_animation->getPose(m_time, m_linearInterpolation, m_indexHint, pose);

	calculatePoseTransforms(
		skeleton,
		&pose,
		outPoseTransforms
	);

	m_time += deltaTime;

	if (m_time >= m_animation->getLastKeyPose().at)
		m_time = 0.0f;

	return true;
}

void SimpleAnimationController::estimateVelocities(
	const Skeleton* skeleton,
	AlignedVector< Velocity >& outVelocities
)
{
}

	}
}
