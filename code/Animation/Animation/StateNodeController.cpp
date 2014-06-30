#include "Animation/SkeletonUtils.h"
#include "Animation/Animation/StateNode.h"
#include "Animation/Animation/StateNodeController.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.StateNodeController", StateNodeController, IPoseController)

StateNodeController::StateNodeController(StateNode* node)
:	m_node(node)
,	m_initialized(false)
{
}

void StateNodeController::destroy()
{
	m_node = 0;
}

void StateNodeController::setTransform(const Transform& transform)
{
}

bool StateNodeController::evaluate(
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	AlignedVector< Transform >& outPoseTransforms,
	bool& outUpdateController
)
{
	Pose currentPose;

	if (!m_initialized)
	{
		if (!m_node || !m_node->prepareContext(m_context))
			return false;
		m_initialized = true;
	}

	m_node->evaluate(
		m_context,
		currentPose
	);

	m_context.setTime(m_context.getTime() + deltaTime);

	float timeLeft = m_context.getDuration() - m_context.getTime();
	if (timeLeft <= 0.0f)
		m_initialized = false;

	calculatePoseTransforms(
		skeleton,
		&currentPose,
		outPoseTransforms
	);

	return true;
}

void StateNodeController::estimateVelocities(
	const Skeleton* skeleton,
	AlignedVector< Velocity >& outVelocities
)
{
}

	}
}
