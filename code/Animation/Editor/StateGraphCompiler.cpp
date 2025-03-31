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
#include "Animation/Editor/StateNode.h"
#include "Animation/Editor/StateTransition.h"
#include "Core/Containers/SmallMap.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.StateGraphCompiler", StateGraphCompiler, Object)

Ref< RtStateGraphData > StateGraphCompiler::compile(const StateGraph* stateGraph) const
{
	SmallMap< const StateNode*, RtStateData* > stateToRts;

	Ref< RtStateGraphData > rtsg = new RtStateGraphData();

	for (auto state : stateGraph->getStates())
	{
		Ref< RtStateData > rts = new RtStateData();
		rts->m_animation = state->getAnimation();
		rtsg->m_states.push_back(rts);
		stateToRts.insert(state, rts);
	}

	rtsg->m_root = stateToRts[stateGraph->getRootState()];
	if (!rtsg->m_root)
		return nullptr;

	for (auto transition : stateGraph->getTransitions())
	{
		Ref< RtStateTransitionData > rtt = new RtStateTransitionData();
		rtt->m_from = stateToRts[transition->from()];
		rtt->m_to = stateToRts[transition->to()];
		if (!rtt->m_from || !rtt->m_to)
			return nullptr;
		rtsg->m_transitions.push_back(rtt);
	}

	return rtsg;
}

}
