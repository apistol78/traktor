/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/Animation.h"
#include "Animation/Editor/StateNode.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"
#include "Resource/Member.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.StateNode", 0, StateNode, ISerializable)

StateNode::StateNode(const std::wstring& name, const resource::Id< Animation >& animation)
:	m_name(name)
,	m_position(0, 0)
,	m_animation(animation)
{
}

const std::wstring& StateNode::getName() const
{
	return m_name;
}

void StateNode::setPosition(const std::pair< int, int >& position)
{
	m_position = position;
}

const std::pair< int, int >& StateNode::getPosition() const
{
	return m_position;
}

void StateNode::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberStlPair< int, int >(L"position", m_position);
	s >> resource::Member< Animation >(L"animation", m_animation);
}

}
