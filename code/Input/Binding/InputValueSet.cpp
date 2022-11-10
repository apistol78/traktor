/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Input/Binding/InputValueSet.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputValueSet", InputValueSet, Object)

void InputValueSet::set(handle_t valueId, float value)
{
	m_valueMap[valueId] = value;
}

float InputValueSet::get(handle_t valueId) const
{
	SmallMap< handle_t, float >::const_iterator i = m_valueMap.find(valueId);
	return i != m_valueMap.end() ? i->second : 0.0f;
}

	}
}
