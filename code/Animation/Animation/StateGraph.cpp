#include <algorithm>
#include "Animation/Animation/StateGraph.h"
#include "Animation/Animation/State.h"
#include "Animation/Animation/Transition.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.animation.StateGraph", StateGraph, Serializable)

void StateGraph::addState(State* state)
{
	m_states.push_back(state);
}

void StateGraph::removeState(State* state)
{
	RefArray< State >::iterator i = std::find(m_states.begin(), m_states.end(), state);
	m_states.erase(i);
}

const RefArray< State >& StateGraph::getStates() const
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

void StateGraph::setRootState(State* rootState)
{
	m_rootState = rootState;
}

Ref< State > StateGraph::getRootState() const
{
	return m_rootState;
}

bool StateGraph::serialize(Serializer& s)
{
	s >> MemberRefArray< State >(L"states", m_states);
	s >> MemberRefArray< Transition >(L"transitions", m_transitions);
	s >> MemberRef< State >(L"rootState", m_rootState);
	return true;
}

	}
}
