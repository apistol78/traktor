#include "Animation/Animation/Animation.h"
#include "Animation/Animation/StateNodeAnimation.h"
#include "Animation/Animation/StateContext.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/IResourceManager.h"
#include "Resource/MemberIdProxy.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.StateNodeAnimation", 0, StateNodeAnimation, StateNode)

StateNodeAnimation::StateNodeAnimation()
{
}

StateNodeAnimation::StateNodeAnimation(const std::wstring& name, const resource::IdProxy< Animation >& animation)
:	StateNode(name)
,	m_animation(animation)
{
}

bool StateNodeAnimation::bind(resource::IResourceManager* resourceManager)
{
	return resourceManager->bind(m_animation);
}

bool StateNodeAnimation::prepareContext(StateContext& outContext)
{
	int poseCount = m_animation->getKeyPoseCount();
	if (poseCount <= 0)
		return false;

	float duration = m_animation->getKeyPose(poseCount - 1).at;

	outContext.setTime(0.0f);
	outContext.setDuration(duration);

	return true;
}

void StateNodeAnimation::evaluate(
	const StateContext& context,
	Pose& outPose
)
{
	float time = context.getTime();
	m_animation->getPose(time, outPose);
}

bool StateNodeAnimation::serialize(ISerializer& s)
{
	if (!StateNode::serialize(s))
		return false;
	s >> resource::MemberIdProxy< Animation >(L"animation", m_animation);
	return true;
}

	}
}
