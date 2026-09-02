/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/StateGraphCompiler.h"

#include "Animation/Animation/RtStateData.h"
#include "Animation/Animation/RtStateGraphData.h"
#include "Animation/Animation/RtStateTransitionData.h"
#include "Animation/Editor/StateGraph.h"
#include "Animation/Editor/StateNodeAnimation.h"
#include "Animation/Editor/StateNodeAny.h"
#include "Animation/Editor/StateNodeController.h"
#include "Animation/Editor/StateTransition.h"
#include "Core/Containers/SmallMap.h"

namespace traktor::animation
{
namespace
{

typedef SmallMap< const StateNode*, AlignedVector< int32_t > > state_indices_t;

template < typename ValueType >
int32_t addUnique(AlignedVector< ValueType >& inoutArray, const ValueType& value)
{
	const auto it = std::find(inoutArray.begin(), inoutArray.end(), value);
	if (it != inoutArray.end())
		return (int32_t)std::distance(inoutArray.begin(), it);

	inoutArray.push_back(value);
	return (int32_t)inoutArray.size() - 1;
}

/*! Get indices of the runtime states which a state was expanded into. */
const AlignedVector< int32_t >& getIndices(const state_indices_t& stateToIndices, const StateNode* state)
{
	static const AlignedVector< int32_t > c_none;
	const auto it = stateToIndices.find(state);
	return it != stateToIndices.end() ? it->second : c_none;
}

/*! Determine if an unconditional transition leave the state when the animation end. */
bool haveEndTransition(const StateGraph* stateGraph, const StateNode* state)
{
	for (auto transition : stateGraph->getTransitions())
	{
		// A transition from an "any" state leave every other state as well.
		if (transition->from() != state && !is_a< StateNodeAny >(transition->from()))
			continue;
		if (transition->getMoment() != Moment::End || !transition->getCondition().empty())
			continue;

		return true;
	}
	return false;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.StateGraphCompiler", StateGraphCompiler, Object)

Ref< RtStateGraphData > StateGraphCompiler::compile(const StateGraph* stateGraph) const
{
	state_indices_t stateToIndices;

	Ref< RtStateGraphData > rtsg = new RtStateGraphData();

	// Create runtime states; an animation state is expanded into one runtime state
	// per animation so the graph randomly select which animation to play.
	for (auto state : stateGraph->getStates())
	{
		if (const auto stateAnimation = dynamic_type_cast< const StateNodeAnimation* >(state))
		{
			AlignedVector< int32_t > indices;
			for (const auto& animation : stateAnimation->getAnimations())
			{
				Ref< RtStateData > rts = new RtStateData();
				rts->m_animation = animation;
				rts->m_transformTime = stateAnimation->getTransformTime();
				rtsg->m_states.push_back(rts);
				indices.push_back((int32_t)rtsg->m_states.size() - 1);
			}
			stateToIndices.insert(state, indices);
		}
		else if (const auto stateController = dynamic_type_cast< const StateNodeController* >(state))
		{
			Ref< RtStateData > rts = new RtStateData();
			rts->m_poseController = stateController->getPoseController();
			rtsg->m_states.push_back(rts);
			stateToIndices.insert(state, { (int32_t)rtsg->m_states.size() - 1 });
		}
	}

	const AlignedVector< int32_t >& rootIndices = getIndices(stateToIndices, stateGraph->getRootState());
	if (!rootIndices.empty())
		rtsg->m_root = rootIndices.front();

	for (auto transition : stateGraph->getTransitions())
	{
		AlignedVector< RtStateTransitionData::Condition > conditions;
		const std::wstring& condition = transition->getCondition();
		if (!condition.empty())
		{
			if (condition[0] == L'!')
			{
				const int32_t index = addUnique(rtsg->m_parameters, condition.substr(1));
				conditions.push_back({ index, true });
			}
			else
			{
				const int32_t index = addUnique(rtsg->m_parameters, condition);
				conditions.push_back({ index, false });
			}
		}

		// Collect all runtime states the transition leave.
		AlignedVector< int32_t > from;
		if (!is_a< StateNodeAny >(transition->from()))
			from = getIndices(stateToIndices, transition->from());
		else
		{
			// Since from is "any" node we need to create a transition from every other state.
			for (auto state : stateGraph->getStates())
			{
				if (!is_a< StateNodeAny >(state))
				{
					const AlignedVector< int32_t >& indices = getIndices(stateToIndices, state);
					from.insert(from.end(), indices.begin(), indices.end());
				}
			}
		}

		// Transition into every runtime state of the target since the graph randomly
		// select one of the equally valid transitions.
		const AlignedVector< int32_t >& to = getIndices(stateToIndices, transition->to());
		for (int32_t f : from)
		{
			for (int32_t t : to)
			{
				Ref< RtStateTransitionData > rtt = new RtStateTransitionData();
				rtt->m_from = f;
				rtt->m_to = t;
				rtt->m_moment = transition->getMoment();
				rtt->m_duration = transition->getDuration();
				rtt->m_conditions = conditions;
				rtsg->m_transitions.push_back(rtt);
			}
		}
	}

	// Create looping transitions for animation states which aren't already left
	// unconditionally when the animation end; without those the state would keep
	// replaying the same animation instead of selecting a new one.
	for (auto state : stateGraph->getStates())
	{
		const auto stateAnimation = dynamic_type_cast< const StateNodeAnimation* >(state);
		if (!stateAnimation || haveEndTransition(stateGraph, state))
			continue;

		const AlignedVector< int32_t >& indices = getIndices(stateToIndices, state);
		for (int32_t f : indices)
		{
			for (int32_t t : indices)
			{
				Ref< RtStateTransitionData > rtt = new RtStateTransitionData();
				rtt->m_from = f;
				rtt->m_to = t;
				rtt->m_moment = Moment::End;
				rtt->m_duration = stateAnimation->getLoopDuration();
				rtsg->m_transitions.push_back(rtt);
			}
		}
	}

	return rtsg;
}

}
