#include "Animation/Animation/State.h"
#include "Animation/Animation/StateContext.h"
#include "Animation/Animation/Animation.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.State", State, ISerializable)

State::State()
:	m_position(0, 0)
{
}

State::State(const std::wstring& name, const resource::Proxy< Animation >& animation)
:	m_name(name)
,	m_position(0, 0)
,	m_animation(animation)
{
}

const std::wstring& State::getName() const
{
	return m_name;
}

void State::setPosition(const std::pair< int, int >& position)
{
	m_position = position;
}

const std::pair< int, int >& State::getPosition() const
{
	return m_position;
}

bool State::prepareContext(StateContext& outContext)
{
	if (!m_animation.validate())
		return false;

	int poseCount = m_animation->getKeyPoseCount();
	if (poseCount <= 0)
		return false;

	float duration = m_animation->getKeyPose(poseCount - 1).at;

	outContext.setTime(0.0f);
	outContext.setDuration(duration);

	return true;
}

void State::evaluate(
	const StateContext& context,
	Pose& outPose
)
{
	if (!m_animation.validate())
		return;

	float time = context.getTime();
	m_animation->getPose(time, outPose);
}

bool State::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberStlPair< int, int >(L"position", m_position);
	s >> resource::Member< Animation >(L"animation", m_animation);
	return true;
}

	}
}
