/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/RtStateTransitionData.h"

#include "Animation/Animation/RtStateData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.RtStateTransitionData", 0, RtStateTransitionData, ISerializable)

void RtStateTransitionData::serialize(ISerializer& s)
{
	s >> MemberRef< RtStateData >(L"from", m_from);
	s >> MemberRef< RtStateData >(L"to", m_to);
}

}
