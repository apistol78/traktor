/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/Transition.h"
#include "Animation/Animation/StateNode.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberEnum.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.Transition", 0, Transition, ISerializable)

Transition::Transition(StateNode* from, StateNode* to)
:	m_from(from)
,	m_to(to)
{
}

StateNode* Transition::from() const
{
	return m_from;
}

StateNode* Transition::to() const
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

void Transition::serialize(ISerializer& s)
{
	const MemberEnum< Moment >::Key c_Moment_Keys[] =
	{
		{ L"TmImmediatly", TmImmediatly },
		{ L"TmEnd", TmEnd },
		{ 0 }
	};

	s >> MemberRef< StateNode >(L"from", m_from);
	s >> MemberRef< StateNode >(L"to", m_to);
	s >> Member< float >(L"duration", m_duration, AttributeRange(0.0f));
	s >> MemberEnum< Moment >(L"moment", m_moment, c_Moment_Keys);
	s >> Member< std::wstring >(L"condition", m_condition);
}

	}
}
