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
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.RtStateGraphData", 0, RtStateGraphData, ISerializable)

Ref< RtStateGraph > RtStateGraphData::createInstance(resource::IResourceManager* resourceManager) const
{
	Ref< RtStateGraph > instance = new RtStateGraph();

	// Create state instances.
	instance->m_states.reserve(m_states.size());
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

	// Initialize parameters.
	instance->m_values.resize(m_parameters.size(), false);
	instance->m_parameters.reserve(m_parameters.size());
	for (int32_t i = 0; i < (int32_t)m_parameters.size(); ++i)
		instance->m_parameters.insert(render::Handle(m_parameters[i].c_str()), i);

	// Create transition instances.
	instance->m_transitions.reserve(m_transitions.size());
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
	s >> MemberAlignedVector< std::wstring >(L"parameters", m_parameters);
	s >> Member< int32_t >(L"root", m_root);
}

}
