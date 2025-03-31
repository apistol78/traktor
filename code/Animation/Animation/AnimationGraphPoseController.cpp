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
	, m_transformTime(transformTime)
	, m_blendState(0.0f)
	, m_blendDuration(0.0f)
	, m_timeFactor(1.0f)
{
}

bool AnimationGraphPoseController::setState(const std::wstring& stateName)
{
	// if (!m_stateGraph)
	//	return false;

	// if (m_currentState && m_currentState->getName() == stateName)
	//	return true;

	// for (auto state : m_stateGraph->getStates())
	//{
	//	if (state->getName() == stateName)
	//	{
	//		m_currentState = state;
	//		m_currentState->prepareContext(m_currentStateContext);
	//		m_nextState = nullptr;
	//		m_blendState = 0.0f;
	//		m_blendDuration = 0.0f;
	//		return true;
	//	}
	// }

	return false;
}

void AnimationGraphPoseController::setCondition(const std::wstring& condition, bool enabled, bool reset)
{
	m_conditions[condition].first = enabled;
	m_conditions[condition].second = reset;
}

void AnimationGraphPoseController::setTime(float time)
{
	T_FATAL_ASSERT(m_currentState);
	m_currentStateContext.setTime(time);
}

float AnimationGraphPoseController::getTime() const
{
	return m_currentStateContext.getTime();
}

void AnimationGraphPoseController::setTimeFactor(float timeFactor)
{
	m_timeFactor = timeFactor;
}

float AnimationGraphPoseController::getTimeFactor() const
{
	return m_timeFactor;
}

void AnimationGraphPoseController::destroy()
{
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
