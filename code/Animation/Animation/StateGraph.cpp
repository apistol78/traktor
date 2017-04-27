/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Animation/Animation/StateGraph.h"
#include "Animation/Animation/StateNode.h"
#include "Animation/Animation/Transition.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.StateGraph", 0, StateGraph, ISerializable)

void StateGraph::addState(StateNode* state)
{
	m_states.push_back(state);
}

void StateGraph::removeState(StateNode* state)
{
	RefArray< StateNode >::iterator i = std::find(m_states.begin(), m_states.end(), state);
	m_states.erase(i);
}

const RefArray< StateNode >& StateGraph::getStates() const
{
	return m_states;
}

void StateGraph::addTransition(Transition* transition)
{
	m_transitions.push_back(transition);
}

void StateGraph::removeTransition(Transition* transition)
{
	RefArray< Transition >::iterator i = std::find(m_transitions.begin(), m_transitions.end(), transition);
	m_transitions.erase(i);
}

const RefArray< Transition >& StateGraph::getTransitions() const
{
	return m_transitions;
}

void StateGraph::setRootState(StateNode* rootState)
{
	m_rootState = rootState;
}

Ref< StateNode > StateGraph::getRootState() const
{
	return m_rootState;
}

void StateGraph::serialize(ISerializer& s)
{
	s >> MemberRefArray< StateNode >(L"states", m_states);
	s >> MemberRefArray< Transition >(L"transitions", m_transitions);
	s >> MemberRef< StateNode >(L"rootState", m_rootState);
}

	}
}
