#include "Animation/SkeletonUtils.h"
#include "Animation/Animation/Animation.h"
#include "Animation/Animation/SimpleAnimationController.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.SimpleAnimationController", SimpleAnimationController, IPoseController)

SimpleAnimationController::SimpleAnimationController(const resource::Proxy< Animation >& animation, bool linearInterpolation)
:	m_animation(animation)
,	m_linearInterpolation(linearInterpolation)
,	m_indexHint(-1)
{
}

void SimpleAnimationController::destroy()
{
}

void SimpleAnimationController::setTransform(const Transform& transform)
{
}

bool SimpleAnimationController::evaluate(
	float time,
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	AlignedVector< Transform >& outPoseTransforms,
	bool& outUpdateController
)
{
	if (!m_animation)
		return false;

	time = std::fmod(time, m_animation->getLastKeyPose().at);

	Pose pose;
	m_animation->getPose(time, m_linearInterpolation, m_indexHint, pose);

	calculatePoseTransforms(
		skeleton,
		&pose,
		outPoseTransforms
	);

	return true;
}

void SimpleAnimationController::estimateVelocities(
	const Skeleton* skeleton,
	AlignedVector< Velocity >& outVelocities
)
{
}

	}
}
