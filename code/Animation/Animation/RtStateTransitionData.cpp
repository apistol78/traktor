/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/RtStateTransitionData.h"

#include "Animation/Animation/RtStateGraph.h"
#include "Animation/Animation/RtStateTransition.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.RtStateTransitionData", 0, RtStateTransitionData, ISerializable)

Ref< RtStateTransition > RtStateTransitionData::createInstance(RtStateGraph* stateGraph) const
{
	Ref< RtStateTransition > instance = new RtStateTransition();

	instance->m_from = stateGraph->m_states[m_from];
	instance->m_to = stateGraph->m_states[m_to];
	instance->m_moment = m_moment;
	instance->m_duration = m_duration;

	return instance;
}

void RtStateTransitionData::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"from", m_from);
	s >> Member< int32_t >(L"to", m_to);
	s >> MemberEnumByValue< Moment >(L"moment", m_moment);
	s >> Member< float >(L"duration", m_duration);
}

}
