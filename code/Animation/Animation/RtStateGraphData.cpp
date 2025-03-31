/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/RtStateGraphData.h"

#include "Animation/Animation/RtStateData.h"
#include "Animation/Animation/RtStateGraph.h"
#include "Animation/Animation/RtStateTransition.h"
#include "Animation/Animation/RtStateTransitionData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.RtStateGraphData", 0, RtStateGraphData, ISerializable)

Ref< RtStateGraph > RtStateGraphData::createInstance(resource::IResourceManager* resourceManager) const
{
	Ref< RtStateGraph > instance = new RtStateGraph();

	// Create state instances.
	for (auto stateData : m_states)
	{
		Ref< RtState > state = stateData->createInstance(resourceManager);
		if (state)
		{
			instance->m_states.push_back(state);
			if (m_root >= 0 && stateData == m_states[m_root])
				instance->m_rootState = state;
		}
		else
			return nullptr;
	}

	// Create transition instances.
	for (auto transitionData : m_transitions)
	{
		Ref< RtStateTransition > transition = transitionData->createInstance(instance);
		if (transition)
			instance->m_transitions.push_back(transition);
		else
			return nullptr;
	}

	return instance;
}

void RtStateGraphData::serialize(ISerializer& s)
{
	s >> MemberRefArray< RtStateData >(L"states", m_states);
	s >> MemberRefArray< RtStateTransitionData >(L"transitions", m_transitions);
	s >> Member< int32_t >(L"root", m_root);
}

}
