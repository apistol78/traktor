/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/RtStateTransition.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.RtStateTransition", RtStateTransition, Object)

RtStateTransition::RtStateTransition(RtState* from, RtState* to)
	: m_from(from)
	, m_to(to)
{
}

bool RtStateTransition::conditionSatisfied(const bool* values) const
{
	for (const Condition& condition : m_conditions)
		if (values[condition.parameter] ^ condition.inverted)
			return true;
	return false;
}

}
