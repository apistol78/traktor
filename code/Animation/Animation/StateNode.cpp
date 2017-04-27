/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/Animation/StateNode.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.StateNode", StateNode, ISerializable)

StateNode::StateNode()
:	m_position(0, 0)
{
}

StateNode::StateNode(const std::wstring& name)
:	m_name(name)
,	m_position(0, 0)
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
}

	}
}
