/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/StateNode.h"
#include "Animation/Editor/StateTransition.h"
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberEnum.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.StateTransition", 0, StateTransition, ISerializable)

StateTransition::StateTransition(StateNode* from, StateNode* to)
:	m_from(from)
,	m_to(to)
{
}

StateNode* StateTransition::from() const
{
	return m_from;
}

StateNode* StateTransition::to() const
{
	return m_to;
}

StateTransition::Moment StateTransition::getMoment() const
{
	return m_moment;
}

float StateTransition::getDuration() const
{
	return m_duration;
}

const std::wstring& StateTransition::getCondition() const
{
	return m_condition;
}

void StateTransition::serialize(ISerializer& s)
{
	s >> MemberRef< StateNode >(L"from", m_from, AttributePrivate());
	s >> MemberRef< StateNode >(L"to", m_to, AttributePrivate());
	s >> Member< float >(L"duration", m_duration, AttributeRange(0.0f) | AttributeUnit(UnitType::Seconds));

	const MemberEnum< Moment >::Key c_Moment_Keys[] =
	{
		{ L"Immediatly", Moment::Immediatly },
		{ L"End", Moment::End },
		{ 0 }
	};
	s >> MemberEnum< Moment >(L"moment", m_moment, c_Moment_Keys);

	s >> Member< std::wstring >(L"condition", m_condition);
}

}
