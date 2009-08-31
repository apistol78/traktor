#include "Animation/Animation/StatePoseController.h"
#include "Animation/Animation/StateGraph.h"
#include "Animation/Animation/State.h"
#include "Animation/Animation/Transition.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/Bone.h"
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

void StatePoseController::setCondition(const std::wstring& condition, bool enabled)
{
	m_conditions[condition] = enabled;
}

void StatePoseController::setTimeFactor(float timeFactor)
{
	m_timeFactor = timeFactor;
}

void StatePoseController::evaluate(
	float deltaTime,
	const Matrix44& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Matrix44 >& boneTransforms,
	AlignedVector< Matrix44 >& outPoseTransforms,
	bool& outUpdateController
)
{
	Pose currentPose;

	// Validate state graph.
	if (!m_stateGraph.valid() || !m_currentState)
	{
		if (!m_stateGraph.validate() || !m_stateGraph->getRootState())
			return;

		// Reset evaluation of graph.
		m_currentState = m_stateGraph->getRootState();
		m_currentState->prepareContext(m_currentStateContext);
		m_nextState = 0;
		m_blendState = 0.0f;
		m_blendDuration = 0.0f;
	}

	if (!m_currentState)
		return;

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

			std::wstring condition = (*i)->getCondition();
			if (condition[0] == L'!')
				value = !m_conditions[condition.substr(1)];
			else
				value = m_conditions[condition];

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
}

void StatePoseController::estimateVelocities(
	const Skeleton* skeleton,
	AlignedVector< Velocity >& outVelocities
)
{
	if (!m_currentState)
		return;

	const Scalar c_deltaTime = Scalar(1.0f / 30.0f);
	AlignedVector< Matrix44 > poseTransforms0, poseTransforms1;
	Pose pose0, pose1;

	float time = m_currentStateContext.getTime();
	m_currentState->evaluate(
		m_currentStateContext,
		pose0
	);
	m_currentStateContext.setTime(time + c_deltaTime);
	m_currentState->evaluate(
		m_currentStateContext,
		pose1
	);
	m_currentStateContext.setTime(time);

	calculatePoseTransforms(skeleton, &pose0, poseTransforms0);
	calculatePoseTransforms(skeleton, &pose1, poseTransforms1);
	T_ASSERT (poseTransforms0.size() == poseTransforms1.size());

	for (uint32_t i = 0; i < uint32_t(poseTransforms0.size()); ++i)
	{
		Vector4 centerOfMass = Vector4(0.0f, 0.0f, skeleton->getBone(i)->getLength() / 2.0f, 1.0f);

		const Vector4& p1 = poseTransforms0[i] * centerOfMass;
		const Vector4& p2 = poseTransforms1[i] * centerOfMass;
		Quaternion q1(poseTransforms0[i]);
		Quaternion q2(poseTransforms1[i]);

		Vector4 dp = p2 - p1;
		Quaternion dq = (q1.inverse() * q2).normalized();

		Velocity velocity;
		velocity.linear = dp / c_deltaTime;
		velocity.angular = dq.toAxisAngle() / c_deltaTime;
		outVelocities.push_back(velocity);
	}
}

	}
}
