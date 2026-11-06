/*
 * TRAKTOR
 * Copyright (c) 2025-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/StateGraph/RtStateGraph.h"

#include "Animation/SkeletonUtils.h"
#include "Animation/StateGraph/RtStateNode.h"
#include "Animation/StateGraph/RtStateTransition.h"
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
	return m_context.setParameterValue(handle, value);
}

bool RtStateGraph::getParameterValue(const render::Handle& handle) const
{
	return m_context.getParameterValue(handle);
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
			m_currentState->reset(worldTransform, skeleton, jointTransforms);
		m_nextState = nullptr;
		m_blendState = 0.0f;
		m_blendDuration = 0.0f;
	}

	if (!m_currentState)
		return false;

	// Evaluate current state.
	Vector4 currentRootMotion = Vector4::zero();
	float currentTimeLeft = m_currentState->evaluate(
		m_context,
		time,
		deltaTime * m_timeFactor,
		worldTransform,
		skeleton,
		jointTransforms,
		m_evaluatePose,
		&currentRootMotion);

	// Build final pose transforms.
	if (m_nextState)
	{
		float nextTimeLeft = 0.0f;

		// Only blend between states if there is a transition time.
		if (m_blendDuration > 0.0f)
		{
			Pose nextPose, blendPose;

			Vector4 nextRootMotion = Vector4::zero();
			nextTimeLeft = m_nextState->evaluate(
				m_context,
				time,
				deltaTime * m_timeFactor,
				worldTransform,
				skeleton,
				jointTransforms,
				nextPose,
				&nextRootMotion);

			const Scalar blend = Scalar(easeInOutCubic(m_blendState / m_blendDuration));

			// Blend root motion with the same weight as the poses.
			m_rootMotion += currentRootMotion * (1.0_simd - blend) + nextRootMotion * blend;
			currentRootMotion = Vector4::zero();

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

		m_rootMotion += currentRootMotion;

		// Swap in next state when we've completely blended into it.
		m_blendState += wallDeltaTime;
		if (m_blendState >= m_blendDuration)
		{
			// Transitions below are now taken from the next state, so use its time left.
			currentTimeLeft = nextTimeLeft;
			m_currentState = m_nextState;
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

		m_rootMotion += currentRootMotion;
	}

	// Execute transition to another state.
	if (!m_nextState)
	{
		RtStateTransition* selectedTransition = nullptr;
		RefArray< RtStateTransition > candidateTransitions;

		// First try all transitions with explicit condition.
		for (auto transition : m_transitions)
		{
			if (transition->getFrom() != m_currentState || !transition->haveConditions())
				continue;

			// Is transition permitted?
			if (!transitionPermitted(transition, currentTimeLeft))
				continue;

			// Is condition satisfied?
			if (!transition->conditionSatisfied(m_context.getValues()))
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
				if (transitionPermitted(transition, currentTimeLeft))
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
		if (selectedTransition == nullptr && currentTimeLeft <= FUZZY_EPSILON)
			selectedTransition = new RtStateTransition(m_currentState, m_currentState);

		// Begin transition to found state; without a transition time it is a cut, taken at once.
		if (selectedTransition != nullptr)
		{
			if (selectedTransition->getDuration() > FUZZY_EPSILON)
			{
				m_nextState = selectedTransition->getTo();
				m_nextState->reset(worldTransform, skeleton, outPoseTransforms);
				m_blendState = 0.0f;
				m_blendDuration = selectedTransition->getDuration();
			}
			else
			{
				cut(selectedTransition->getTo(), worldTransform, skeleton, outPoseTransforms);
				continous = false;
			}
		}
	}
	else
	{
		for (auto transition : m_transitions)
		{
			if (transition->getFrom() != m_nextState || !transition->haveConditions() || transition->getDuration() > FUZZY_EPSILON)
				continue;
			if (!transition->conditionSatisfied(m_context.getValues()))
				continue;

			cut(transition->getTo(), worldTransform, skeleton, outPoseTransforms);
			continous = false;
			break;
		}
	}

	return continous;
}

void RtStateGraph::cut(RtStateNode* state, const Transform& worldTransform, const Skeleton* skeleton, const AlignedVector< Transform >& poseTransforms)
{
	m_currentState = state;
	m_currentState->reset(worldTransform, skeleton, poseTransforms);
	m_nextState = nullptr;
	m_blendState = 0.0f;
	m_blendDuration = 0.0f;
}

Vector4 RtStateGraph::consumeRootMotion()
{
	const Vector4 rootMotion = m_rootMotion;
	m_rootMotion = Vector4::zero();
	return rootMotion;
}

void RtStateGraph::getPoseControllersOf(const TypeInfo& type, bool activeOnly, RefArray< IPoseController >& outControllers) const
{
	if (activeOnly)
	{
		if (m_currentState != nullptr)
			m_currentState->getPoseControllersOf(type, true, outControllers);
		if (m_nextState != nullptr)
			m_nextState->getPoseControllersOf(type, true, outControllers);
	}
	else
	{
		for (auto state : m_states)
			state->getPoseControllersOf(type, false, outControllers);
	}
}

}
