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
#include "Animation/Editor/StateTransition.h"
#include "Core/Containers/SmallMap.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.StateGraphCompiler", StateGraphCompiler, Object)

Ref< RtStateGraphData > StateGraphCompiler::compile(const StateGraph* stateGraph) const
{
	SmallMap< const StateNode*, int32_t > stateToIndex;

	Ref< RtStateGraphData > rtsg = new RtStateGraphData();

	for (auto state : stateGraph->getStates())
		if (const auto stateAnimation = dynamic_type_cast< const StateNodeAnimation* >(state))
		{
			Ref< RtStateData > rts = new RtStateData();
			rts->m_animation = stateAnimation->getAnimation();
			rtsg->m_states.push_back(rts);
			stateToIndex.insert(state, (int32_t)rtsg->m_states.size() - 1);
		}

	rtsg->m_root = stateToIndex[stateGraph->getRootState()];

	for (auto transition : stateGraph->getTransitions())
	{
		if (!is_a< StateNodeAny >(transition->from()))
		{
			Ref< RtStateTransitionData > rtt = new RtStateTransitionData();
			rtt->m_from = stateToIndex[transition->from()];
			rtt->m_to = stateToIndex[transition->to()];
			rtt->m_moment = transition->getMoment();
			rtt->m_duration = transition->getDuration();
			rtsg->m_transitions.push_back(rtt);
		}
		else
		{
			// Since from is "any" node we need to create a transition from every transition.
			for (auto state : stateGraph->getStates())
			{
				if (!is_a< StateNodeAny >(state))
				{
					Ref< RtStateTransitionData > rtt = new RtStateTransitionData();
					rtt->m_from = stateToIndex[state];
					rtt->m_to = stateToIndex[transition->to()];
					rtt->m_moment = transition->getMoment();
					rtt->m_duration = transition->getDuration();
					rtsg->m_transitions.push_back(rtt);
				}
			}
		}
	}

	return rtsg;
}

}
