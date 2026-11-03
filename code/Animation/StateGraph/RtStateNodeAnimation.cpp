/*
 * TRAKTOR
 * Copyright (c) 2025-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/StateGraph/RtStateNodeAnimation.h"

#include "Animation/Animation.h"
#include "Animation/ITransformTime.h"
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/StateGraph/StateContext.h"
#include "Core/Misc/SafeDestroy.h"

#include <limits>

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.RtStateNodeAnimation", RtStateNodeAnimation, RtStateNode)

void RtStateNodeAnimation::destroy()
{
	m_animation.clear();
	m_transformTime = nullptr;
}

bool RtStateNodeAnimation::prepare(StateContext& outContext) const
{
	if (!m_animation)
		return false;

	const uint32_t poseCount = m_animation->getKeyPoseCount();
	if (poseCount < 1)
		return false;

	const float duration = m_animation->getKeyPose(poseCount - 1).at;

	outContext.setTime(0.0f);
	outContext.setDuration(duration);
	outContext.setRootMotionTime(-1.0f);
	return true;
}

void RtStateNodeAnimation::reset(
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& poseTransforms) const
{
}

void RtStateNodeAnimation::evaluate(
	StateContext& context,
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	Pose& outPose,
	Vector4* outRootMotion) const
{
	float time = context.getTime();

	if (m_transformTime && m_animation)
		m_transformTime->calculateTime(
			m_animation,
			worldTransform,
			time,
			deltaTime
		);

	if (m_animation)
		m_animation->getPose(time, outPose);

	if (outRootMotion)
	{
		*outRootMotion = Vector4::zero();
		if (m_animation && m_animation->haveRootMotion() && !m_transformTime)
		{
			const float lastTime = context.getRootMotionTime() >= 0.0f ? context.getRootMotionTime() : 0.0f;
			if (time > lastTime)
			{
				const Vector4 rm0 = m_animation->getRootMotion(lastTime);
				const Vector4 rm1 = m_animation->getRootMotion(time);
				const Vector4 travel = Quaternion::fromAxisAngle(Vector4(0.0f, 1.0f, 0.0f, 0.0f), -rm0.w()) * (rm1 - rm0).xyz0();
				*outRootMotion = Vector4(travel.x(), 0.0f, travel.z(), rm1.w() - rm0.w());
			}
		}
		context.setRootMotionTime(time);
	}
}

IPoseController* RtStateNodeAnimation::getActivePoseController() const
{
	return nullptr;
}

void RtStateNodeAnimation::getPoseControllersOf(const TypeInfo& type, RefArray< IPoseController >& outControllers) const
{
}

}
