#include "Animation/Animation/Transition.h"
#include "Animation/Animation/State.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberEnum.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.Transition", 0, Transition, ISerializable)

Transition::Transition()
:	m_moment(TmImmediatly)
,	m_duration(0.0f)
{
}

Transition::Transition(State* from, State* to)
:	m_from(from)
,	m_to(to)
,	m_moment(TmImmediatly)
,	m_duration(0.0f)
{
}

Ref< State > Transition::from() const
{
	return m_from;
}

Ref< State > Transition::to() const
{
	return m_to;
}

Transition::Moment Transition::getMoment() const
{
	return m_moment;
}

float Transition::getDuration() const
{
	return m_duration;
}

const std::wstring& Transition::getCondition() const
{
	return m_condition;
}

bool Transition::serialize(ISerializer& s)
{
	const MemberEnum< Moment >::Key c_Moment_Keys[] =
	{
		{ L"TmImmediatly", TmImmediatly },
		{ L"TmEnd", TmEnd },
		{ 0 }
	};

	s >> MemberRef< State >(L"from", m_from);
	s >> MemberRef< State >(L"to", m_to);
	s >> Member< float >(L"duration", m_duration, 0.0f);
	s >> MemberEnum< Moment >(L"moment", m_moment, c_Moment_Keys);
	s >> Member< std::wstring >(L"condition", m_condition);

	return true;
}

	}
}
