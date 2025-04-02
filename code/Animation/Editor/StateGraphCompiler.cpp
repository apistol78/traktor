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

template < typename ValueType >
int32_t addUnique(AlignedVector< ValueType >& inoutArray, const ValueType& value)
{
	const auto it = std::find(inoutArray.begin(), inoutArray.end(), value);
	if (it != inoutArray.end())
		return (int32_t)std::distance(inoutArray.begin(), it);

	inoutArray.push_back(value);
	return (int32_t)inoutArray.size() - 1;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.StateGraphCompiler", StateGraphCompiler, Object)

Ref< RtStateGraphData > StateGraphCompiler::compile(const StateGraph* stateGraph) const
{
	SmallMap< const StateNode*, int32_t > stateToIndex;

	Ref< RtStateGraphData > rtsg = new RtStateGraphData();

	for (auto state : stateGraph->getStates())
	{
		if (const auto stateAnimation = dynamic_type_cast< const StateNodeAnimation* >(state))
		{
			Ref< RtStateData > rts = new RtStateData();
			rts->m_animation = stateAnimation->getAnimation();
			rtsg->m_states.push_back(rts);
			stateToIndex.insert(state, (int32_t)rtsg->m_states.size() - 1);
		}
		else if (const auto stateController = dynamic_type_cast< const StateNodeController* >(state))
		{
			Ref< RtStateData > rts = new RtStateData();
			rts->m_poseController = stateController->getPoseController();
			rtsg->m_states.push_back(rts);
			stateToIndex.insert(state, (int32_t)rtsg->m_states.size() - 1);
		}
	}

	rtsg->m_root = stateToIndex[stateGraph->getRootState()];

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

		if (!is_a< StateNodeAny >(transition->from()))
		{
			Ref< RtStateTransitionData > rtt = new RtStateTransitionData();
			rtt->m_from = stateToIndex[transition->from()];
			rtt->m_to = stateToIndex[transition->to()];
			rtt->m_moment = transition->getMoment();
			rtt->m_duration = transition->getDuration();
			rtt->m_conditions = conditions;
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
					rtt->m_conditions = conditions;
					rtsg->m_transitions.push_back(rtt);
				}
			}
		}
	}

	return rtsg;
}

}
