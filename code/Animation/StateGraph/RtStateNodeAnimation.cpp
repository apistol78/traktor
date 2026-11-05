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

void RtStateNodeAnimation::reset(
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& poseTransforms)
{
	m_duration = 0.0f;
	m_time = 0.0f;
	m_lastTime = 0.0f;

	if (!m_animation)
		return;

	const uint32_t poseCount = m_animation->getKeyPoseCount();
	if (poseCount < 1)
		return;

	m_duration = m_animation->getKeyPose(poseCount - 1).at;
}

float RtStateNodeAnimation::evaluate(
	const RtStateContext& /*context*/,
	float /*time*/,
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	Pose& outPose,
	Vector4* outRootMotion)
{
	float time = m_time;

	if (m_transformTime && m_animation)
		m_transformTime->calculateTime(
			m_animation,
			worldTransform,
			time,
			deltaTime);

	if (m_animation)
		m_animation->getPose(time, outPose);

	if (outRootMotion)
	{
		*outRootMotion = Vector4::zero();
		if (m_animation && m_animation->haveRootMotion())
		{
			if (time > m_lastTime)
			{
				const Vector4 rm0 = m_animation->getRootMotion(m_lastTime);
				const Vector4 rm1 = m_animation->getRootMotion(time);
				const Vector4 travel = Quaternion::fromAxisAngle(Vector4(0.0f, 1.0f, 0.0f, 0.0f), -rm0.w()) * (rm1 - rm0).xyz0();
				*outRootMotion = Vector4(travel.x(), 0.0f, travel.z(), rm1.w() - rm0.w());
			}
		}
	}

	m_lastTime = time;
	m_time += deltaTime;
	return m_duration - m_time;
}

IPoseController* RtStateNodeAnimation::getActivePoseController() const
{
	return nullptr;
}

void RtStateNodeAnimation::getPoseControllersOf(const TypeInfo& type, RefArray< IPoseController >& outControllers) const
{
}
}
