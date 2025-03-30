/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

class StateNode;
class StateTransition;

/*

editor:

StateGraph
StateNode
StateTransition

-->

StateGraphCompiler



RtStateGraphData
RtStateData
RtStateTransitionData


RtStateGraph
RtState
RtStateTransition


*/

/*! Animation state graph.
 * \ingroup Animation
 */
class T_DLLCLASS StateGraph : public ISerializable
{
	T_RTTI_CLASS;

public:
	void addState(StateNode* state);

	void removeState(StateNode* state);

	const RefArray< StateNode >& getStates() const;

	void addTransition(StateTransition* transition);

	void removeTransition(StateTransition* transition);

	const RefArray< StateTransition >& getTransitions() const;

	void setRootState(StateNode* rootState);

	Ref< StateNode > getRootState() const;

	virtual void serialize(ISerializer& s) override final;

private:
	RefArray< StateNode > m_states;
	RefArray< StateTransition > m_transitions;
	Ref< StateNode > m_rootState;
};

}
