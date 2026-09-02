/*
 * TRAKTOR
 * Copyright (c) 2025-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/RtStateGraph.h"

#include "Animation/Animation/RtState.h"
#include "Animation/Animation/RtStateTransition.h"
#include "Animation/SkeletonUtils.h"
#include "Core/Math/Const.h"
#include "Core/Math/Random.h"

namespace traktor::animation
{
namespace
{

Random s_random;

/*! Determine if a transition is permitted at the current time. */
bool transitionPermitted(const RtStateTransition* transition, float timeLeft)
{
	switch (transition->getMoment())
	{
	case Moment::Immediately:
		return true;

	case Moment::End:
		// Slack since the state time accumulate a small error until it reach the duration.
		return timeLeft <= transition->getDuration() + FUZZY_EPSILON;

	default:
		return false;
	}
}

float easeInOutCubic(float f)
{
	if (f < 0.5f)
		return 4.0f * f * f * f;
	const float p = 2.0f * f - 2.0f;
	return 0.5f * p * p * p + 1.0f;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.RtStateGraph", RtStateGraph, Object)

void RtStateGraph::destroy()
{
	for (auto state : m_states)
		state->destroy();
	m_states.clear();
}

bool RtStateGraph::setParameterValue(const render::Handle& handle, bool value)
{
	const auto it = m_parameters.find(handle);
	if (it != m_parameters.end())
	{
		m_values[it->second] = value;
		return true;
	}
	else
		return false;
}

bool RtStateGraph::getParameterValue(const render::Handle& handle) const
{
	const auto it = m_parameters.find(handle);
	if (it != m_parameters.end())
		return m_values[it->second];
	else
		return false;
}

bool RtStateGraph::evaluate(
	float time,
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	AlignedVector< Transform >& outPoseTransforms)
{
	const float wallDeltaTime = deltaTime;
	bool continous = true;

	if (!skeleton)
		return false;

	// Prepare graph evaluation context.
	if (!m_currentState)
	{
		m_currentState = m_rootState;
		if (m_currentState)
		{
			if (!m_currentState->prepare(m_currentStateContext))
				return false;
			// No prior pose on the initial state; initialize from the incoming (bind/rest)
			// joint transforms.
			m_currentState->reset(worldTransform, skeleton, jointTransforms);
		}
		m_nextState = nullptr;
		m_blendState = 0.0f;
		m_blendDuration = 0.0f;
	}

	if (!m_currentState)
		return false;

	// Evaluate current state.
	m_currentState->evaluate(
		m_currentStateContext,
		deltaTime * m_timeFactor,
		worldTransform,
		skeleton,
		jointTransforms,
		m_evaluatePose);
	m_currentStateContext.setTime(m_currentStateContext.getTime() + deltaTime * m_timeFactor);

	// Build final pose transforms.
	if (m_nextState)
	{
		// Only blend between states if there is a transition time.
		if (m_blendDuration > 0.0f)
		{
			Pose nextPose, blendPose;

			m_nextState->evaluate(
				m_nextStateContext,
				deltaTime * m_timeFactor,
				worldTransform,
				skeleton,
				jointTransforms,
				nextPose);
			m_nextStateContext.setTime(m_nextStateContext.getTime() + deltaTime * m_timeFactor);

			const Scalar blend = Scalar(easeInOutCubic(m_blendState / m_blendDuration));

			blendPoses(
				&m_evaluatePose,
				&nextPose,
				blend,
				&blendPose);

			calculatePoseTransforms(
				skeleton,
				&blendPose,
				outPoseTransforms);
		}
		else
		{
			calculatePoseTransforms(
				skeleton,
				&m_evaluatePose,
				outPoseTransforms);
		}

		// Swap in next state when we've completely blended into it.
		m_blendState += wallDeltaTime;
		if (m_blendState >= m_blendDuration)
		{
			m_currentState = m_nextState;
			m_currentStateContext = m_nextStateContext;
			m_nextState = nullptr;
			m_blendState = 0.0f;
			m_blendDuration = 0.0f;
			continous = bool(m_blendDuration > FUZZY_EPSILON);
		}
	}
	else
	{
		calculatePoseTransforms(
			skeleton,
			&m_evaluatePose,
			outPoseTransforms);
	}

	// Execute transition to another state.
	if (!m_nextState)
	{
		const float timeLeft = max(m_currentStateContext.getDuration() - m_currentStateContext.getTime(), 0.0f);
		RtStateTransition* selectedTransition = nullptr;
		RefArray< RtStateTransition > candidateTransitions;

		// First try all transitions with explicit condition.
		for (auto transition : m_transitions)
		{
			if (transition->getFrom() != m_currentState || !transition->haveConditions())
				continue;

			// Is transition permitted?
			if (!transitionPermitted(transition, timeLeft))
				continue;

			// Is condition satisfied?
			if (!transition->conditionSatisfied(m_values.c_ptr()))
				continue;

			candidateTransitions.push_back(transition);
		}

		// Still no transition state found, we try all transitions without explicit condition.
		if (candidateTransitions.empty())
		{
			for (auto transition : m_transitions)
			{
				if (transition->getFrom() != m_currentState || transition->haveConditions())
					continue;

				// Is transition permitted?
				if (transitionPermitted(transition, timeLeft))
					candidateTransitions.push_back(transition);
			}
		}

		// Randomly select one of the found, valid, transitions; a state which have
		// been expanded into several animations have one transition per animation.
		if (!candidateTransitions.empty())
		{
			const uint32_t i = s_random.next() % candidateTransitions.size();
			selectedTransition = candidateTransitions[i];
		}

		// Still no transition, repeat current state if we're at the end.
		if (selectedTransition == nullptr && timeLeft <= FUZZY_EPSILON)
			selectedTransition = new RtStateTransition(m_currentState, m_currentState);

		// Begin transition to found state.
		if (selectedTransition != nullptr)
		{
			m_nextState = selectedTransition->getTo();
			m_nextState->reset(worldTransform, skeleton, outPoseTransforms);
			m_nextState->prepare(m_nextStateContext);
			m_blendState = 0.0f;
			m_blendDuration = selectedTransition->getDuration();
		}
	}

	return continous;
}

IPoseController* RtStateGraph::getActivePoseController() const
{
	return m_currentState ? m_currentState->getActivePoseController() : nullptr;
}

void RtStateGraph::getPoseControllersOf(const TypeInfo& type, RefArray< IPoseController >& outControllers) const
{
	if (m_currentState != nullptr)
		m_currentState->getPoseControllersOf(type, outControllers);
}

}
