/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/AnimationGraphPoseController.h"

#include "Animation/Animation/RtStateGraph.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimationGraphPoseController", AnimationGraphPoseController, IPoseController)

AnimationGraphPoseController::AnimationGraphPoseController(const resource::Proxy< RtStateGraph >& stateGraph, ITransformTime* transformTime)
	: m_stateGraph(stateGraph)
{
}

bool AnimationGraphPoseController::setState(const std::wstring& stateName)
{
	return false;
}

bool AnimationGraphPoseController::setParameterValue(const render::Handle& parameter, bool value)
{
	if (m_stateGraph)
		return m_stateGraph->setParameterValue(parameter, value);
	else
		return false;
}

void AnimationGraphPoseController::setTime(float time)
{
}

float AnimationGraphPoseController::getTime() const
{
	return 0.0f;
}

void AnimationGraphPoseController::setTimeFactor(float timeFactor)
{
}

float AnimationGraphPoseController::getTimeFactor() const
{
	return 1.0f;
}

void AnimationGraphPoseController::destroy()
{
	m_stateGraph.clear();
}

void AnimationGraphPoseController::setTransform(const Transform& transform)
{
}

bool AnimationGraphPoseController::evaluate(
	float time,
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	AlignedVector< Transform >& outPoseTransforms)
{
	return m_stateGraph ? m_stateGraph->evaluate(time, deltaTime, worldTransform, skeleton, jointTransforms, outPoseTransforms) : false;
}

}
