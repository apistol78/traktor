/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/AnimationGraph.h"
#include "Animation/Animation/StateNode.h"
#include "Animation/Animation/Transition.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.AnimationGraph", 0, AnimationGraph, ISerializable)

void AnimationGraph::addState(StateNode* state)
{
	m_states.push_back(state);
}

void AnimationGraph::removeState(StateNode* state)
{
	m_states.remove(state);
}

const RefArray< StateNode >& AnimationGraph::getStates() const
{
	return m_states;
}

void AnimationGraph::addTransition(Transition* transition)
{
	m_transitions.push_back(transition);
}

void AnimationGraph::removeTransition(Transition* transition)
{
	m_transitions.remove(transition);
}

const RefArray< Transition >& AnimationGraph::getTransitions() const
{
	return m_transitions;
}

void AnimationGraph::setRootState(StateNode* rootState)
{
	m_rootState = rootState;
}

Ref< StateNode > AnimationGraph::getRootState() const
{
	return m_rootState;
}

void AnimationGraph::serialize(ISerializer& s)
{
	s >> MemberRefArray< StateNode >(L"states", m_states);
	s >> MemberRefArray< Transition >(L"transitions", m_transitions);
	s >> MemberRef< StateNode >(L"rootState", m_rootState);
}

}
