#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/Animation/StateNode.h"
#include "Animation/Animation/StateGraph.h"
#include "Animation/Animation/StatePoseController.h"
#include "Animation/Animation/Transition.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.StatePoseController", StatePoseController, IPoseController)

StatePoseController::StatePoseController(const resource::Proxy< StateGraph >& stateGraph)
:	m_stateGraph(stateGraph)
,	m_blendState(0.0f)
,	m_blendDuration(0.0f)
,	m_timeFactor(1.0f)
{
}

bool StatePoseController::setState(const std::wstring& stateName)
{
	if (!m_stateGraph)
		return false;

	if (m_currentState && m_currentState->getName() == stateName)
		return true;

	const RefArray< StateNode >& states = m_stateGraph->getStates();
	for (RefArray< StateNode >::const_iterator i = states.begin(); i != states.end(); ++i)
	{
		if ((*i)->getName() == stateName)
		{
			m_currentState = *i;
			m_currentState->prepareContext(m_currentStateContext);
			m_nextState = 0;
			m_blendState = 0.0f;
			m_blendDuration = 0.0f;
			return true;
		}
	}

	return false;
}

void StatePoseController::setCondition(const std::wstring& condition, bool enabled, bool reset)
{
	m_conditions[condition].first = enabled;
	m_conditions[condition].second = reset;
}

void StatePoseController::setTime(float time)
{
	T_FATAL_ASSERT (m_currentState);
	m_currentStateContext.setTime(time);
	m_currentStateContext.setIndexHint(-1);
}

float StatePoseController::getTime() const
{
	return m_currentStateContext.getTime();
}

void StatePoseController::setTimeFactor(float timeFactor)
{
	m_timeFactor = timeFactor;
}

float StatePoseController::getTimeFactor() const
{
	return m_timeFactor;
}

void StatePoseController::destroy()
{
}

void StatePoseController::setTransform(const Transform& transform)
{
}

bool StatePoseController::evaluate(
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	AlignedVector< Transform >& outPoseTransforms,
	bool& outUpdateController
)
{
	bool continous = true;
	Pose currentPose;

	if (!skeleton)
		return false;

	if (m_stateGraph.changed())
	{
		m_currentState = 0;
		m_stateGraph.consume();
	}

	// Prepare graph evaluation context.
	if (!m_currentState)
	{
		m_currentState = m_stateGraph->getRootState();
		if (m_currentState)
		{
			if (!m_currentState->prepareContext(m_currentStateContext))
				return false;
		}
		m_nextState = 0;
		m_blendState = 0.0f;
		m_blendDuration = 0.0f;
	}

	if (!m_currentState)
		return false;

	// Evaluate current state.
	m_currentState->evaluate(
		m_currentStateContext,
		currentPose
	);
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
				nextPose
			);
			m_nextStateContext.setTime(m_nextStateContext.getTime() + deltaTime * m_timeFactor);

			Scalar blend = Scalar(sinf((m_blendState / m_blendDuration) * PI / 2.0f));

			blendPoses(
				&currentPose,
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
				&currentPose,
				outPoseTransforms
			);
		}

		// Swap in next state when we've completely blended into it.
		m_blendState += deltaTime;
		if (m_blendState >= m_blendDuration)
		{
			m_currentState = m_nextState;
			m_currentStateContext = m_nextStateContext;
			m_nextState = 0;
			m_blendState = 0.0f;
			m_blendDuration = 0.0f;
			continous = bool(m_blendDuration > FUZZY_EPSILON);
		}
	}
	else
	{
		calculatePoseTransforms(
			skeleton,
			&currentPose,
			outPoseTransforms
		);
	}

	// Execute transition to another state.
	if (!m_nextState)
	{
		const RefArray< Transition >& transitions = m_stateGraph->getTransitions();

		// First try all transitions with explicit condition.
		for (RefArray< Transition >::const_iterator i = transitions.begin(); i != transitions.end(); ++i)
		{
			if ((*i)->from() != m_currentState || (*i)->getCondition().empty())
				continue;

			// Is transition permitted?
			bool transitionPermitted = false;
			switch ((*i)->getMoment())
			{
			case Transition::TmImmediatly:
				transitionPermitted = true;
				break;

			case Transition::TmEnd:
				{
					float timeLeft = max(m_currentStateContext.getDuration() - m_currentStateContext.getTime(), 0.0f);
					if (timeLeft <= (*i)->getDuration())
						transitionPermitted = true;
				}
				break;
			}
			if (!transitionPermitted)
				continue;

			// Is condition satisfied?
			bool value = false;

			const std::wstring& condition = (*i)->getCondition();
			if (!condition.empty() && condition[0] == L'!')
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

			// Begin transition to found state.
			m_nextState = (*i)->to();
			m_nextState->prepareContext(m_nextStateContext);
			m_blendState = 0.0f;
			m_blendDuration = (*i)->getDuration();
			break;
		}

		// Still no transition state found, we try all transitions without explicit condition.
		for (RefArray< Transition >::const_iterator i = transitions.begin(); i != transitions.end(); ++i)
		{
			if ((*i)->from() != m_currentState || !(*i)->getCondition().empty())
				continue;

			// Is transition permitted?
			bool transitionPermitted = false;
			switch ((*i)->getMoment())
			{
			case Transition::TmImmediatly:
				transitionPermitted = true;
				break;

			case Transition::TmEnd:
				{
					float timeLeft = max(m_currentStateContext.getDuration() - m_currentStateContext.getTime(), 0.0f);
					if (timeLeft <= (*i)->getDuration())
						transitionPermitted = true;
				}
				break;
			}
			if (!transitionPermitted)
				continue;

			// Begin transition to found state.
			m_nextState = (*i)->to();
			m_nextState->prepareContext(m_nextStateContext);
			m_blendState = 0.0f;
			m_blendDuration = (*i)->getDuration();
			break;
		}
	}

	return continous;
}

void StatePoseController::estimateVelocities(
	const Skeleton* skeleton,
	AlignedVector< Velocity >& outVelocities
)
{
}

	}
}
