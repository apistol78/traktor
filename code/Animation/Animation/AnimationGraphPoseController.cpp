/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/Animation/Animation.h"
#include "Animation/Animation/AnimationGraph.h"
#include "Animation/Animation/AnimationGraphPoseController.h"
#include "Animation/Animation/ITransformTime.h"
#include "Animation/Animation/StateNode.h"
#include "Animation/Animation/Transition.h"
#include "Core/Math/Const.h"
#include "Core/Math/Random.h"

namespace traktor::animation
{
	namespace
	{

Random s_random;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimationGraphPoseController", AnimationGraphPoseController, IPoseController)

AnimationGraphPoseController::AnimationGraphPoseController(const resource::Proxy< AnimationGraph >& stateGraph, ITransformTime* transformTime)
:	m_animationGraph(stateGraph)
,	m_transformTime(transformTime)
,	m_blendState(0.0f)
,	m_blendDuration(0.0f)
,	m_timeFactor(1.0f)
{
}

bool AnimationGraphPoseController::setState(const std::wstring& stateName)
{
	if (!m_animationGraph)
		return false;

	if (m_currentState && m_currentState->getName() == stateName)
		return true;

	for (auto state : m_animationGraph->getStates())
	{
		if (state->getName() == stateName)
		{
			m_currentState = state;
			m_currentState->prepareContext(m_currentStateContext);
			m_nextState = nullptr;
			m_blendState = 0.0f;
			m_blendDuration = 0.0f;
			return true;
		}
	}

	return false;
}

void AnimationGraphPoseController::setCondition(const std::wstring& condition, bool enabled, bool reset)
{
	m_conditions[condition].first = enabled;
	m_conditions[condition].second = reset;
}

void AnimationGraphPoseController::setTime(float time)
{
	T_FATAL_ASSERT (m_currentState);
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
	AlignedVector< Transform >& outPoseTransforms
)
{
	const float wallDeltaTime = deltaTime;
	bool continous = true;

	if (!skeleton)
		return false;

	if (m_animationGraph.changed())
	{
		m_currentState = nullptr;
		m_animationGraph.consume();
	}

	// Prepare graph evaluation context.
	if (!m_currentState)
	{
		m_currentState = m_animationGraph->getRootState();
		if (m_currentState)
		{
			if (!m_currentState->prepareContext(m_currentStateContext))
				return false;
		}
		m_nextState = nullptr;
		m_blendState = 0.0f;
		m_blendDuration = 0.0f;
	}

	if (!m_currentState)
		return false;

	// Transform, or remap, time.
	if (m_transformTime && m_currentState)
	{
		const Animation* animation = m_currentState->getAnimation();
		if (animation)
			m_transformTime->calculateTime(animation, worldTransform, time, deltaTime);
	}

	// Evaluate current state.
	m_currentState->evaluate(
		m_currentStateContext,
		m_evaluatePose
	);
	m_currentStateContext.setTime(m_currentStateContext.getTime() + deltaTime * m_timeFactor);

	// Build final pose transforms.
	if (m_nextState)
	{
		// Only blend between states if there is a transition time.
		if (m_blendDuration > 0.0f)
		{
			Pose nextPose, blendPose;

			// Transform, or remap, time.
			if (m_transformTime && m_nextState)
			{
				const Animation* animation = m_nextState->getAnimation();
				if (animation)
					m_transformTime->calculateTime(animation, worldTransform, time, deltaTime);
			}

			m_nextState->evaluate(
				m_nextStateContext,
				nextPose
			);
			m_nextStateContext.setTime(m_nextStateContext.getTime() + deltaTime * m_timeFactor);

			const Scalar blend = Scalar(sinf((m_blendState / m_blendDuration) * PI / 2.0f));

			blendPoses(
				&m_evaluatePose,
				&nextPose,
				blend,
				&blendPose
			);

			calculatePoseTransforms(
				skeleton,
				&blendPose,
				outPoseTransforms
			);
		}
		else
		{
			calculatePoseTransforms(
				skeleton,
				&m_evaluatePose,
				outPoseTransforms
			);
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
			outPoseTransforms
		);
	}

	// Execute transition to another state.
	if (!m_nextState)
	{
		const RefArray< Transition >& transitions = m_animationGraph->getTransitions();
		Transition* selectedTransition = nullptr;

		// First try all transitions with explicit condition.
		for (auto transition : transitions)
		{
			if (transition->from() != m_currentState || transition->getCondition().empty())
				continue;

			// Is transition permitted?
			bool transitionPermitted = false;
			switch (transition->getMoment())
			{
			case Transition::Moment::Immediatly:
				transitionPermitted = true;
				break;

			case Transition::Moment::End:
				{
					const float timeLeft = max(m_currentStateContext.getDuration() - m_currentStateContext.getTime(), 0.0f);
					if (timeLeft <= transition->getDuration())
						transitionPermitted = true;
				}
				break;
			}
			if (!transitionPermitted)
				continue;

			// Is condition satisfied?
			bool value = false;
			const std::wstring& condition = transition->getCondition();
			if (condition[0] == L'!')
			{
				std::pair< bool, bool >& cv = m_conditions[condition.substr(1)];
				value = !cv.first;
				if (value && cv.second)
				{
					cv.first = !cv.first;
					cv.second = false;
				}
			}
			else
			{
				std::pair< bool, bool >& cv = m_conditions[condition];
				value = cv.first;
				if (value && cv.second)
				{
					cv.first = !cv.first;
					cv.second = false;
				}
			}
			if (!value)
				continue;

			// Found valid transition.
			selectedTransition = transition;
			break;
		}

		// Still no transition state found, we try all transitions without explicit condition.
		if (selectedTransition == nullptr)
		{
			RefArray< Transition > candidateTransitions;
			for (auto transition : transitions)
			{
				if (transition->from() != m_currentState || !transition->getCondition().empty())
					continue;

				// Is transition permitted?
				bool transitionPermitted = false;
				switch (transition->getMoment())
				{
				case Transition::Moment::Immediatly:
					transitionPermitted = true;
					break;

				case Transition::Moment::End:
					{
						const float timeLeft = max(m_currentStateContext.getDuration() - m_currentStateContext.getTime(), 0.0f);
						if (timeLeft <= transition->getDuration())
							transitionPermitted = true;
					}
					break;
				}
				if (!transitionPermitted)
					continue;

				candidateTransitions.push_back(transition);
			}

			// Randomly select one of the found, valid, transitions.
			if (!candidateTransitions.empty())
			{
				const uint32_t i = s_random.next() % candidateTransitions.size();
				selectedTransition = candidateTransitions[i];
			}
		}

		// Yet no transition, repeat current state if we're at the end.
		if (selectedTransition == nullptr)
		{
			const float timeLeft = max(m_currentStateContext.getDuration() - m_currentStateContext.getTime(), 0.0f);
			if (timeLeft <= 0.0f)
				selectedTransition = new Transition(m_currentState, m_currentState);
		}

		// Begin transition to found state.
		if (selectedTransition != nullptr)
		{
			m_nextState = selectedTransition->to();
			m_nextState->prepareContext(m_nextStateContext);
			m_blendState = 0.0f;
			m_blendDuration = selectedTransition->getDuration();
		}
	}

	return continous;
}

}
