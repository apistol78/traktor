/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Input/Binding/InputValueSet.h"

namespace traktor::input
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputValueSet", InputValueSet, Object)

void InputValueSet::set(handle_t valueId, float value)
{
	m_valueMap[valueId] = value;
}

float InputValueSet::get(handle_t valueId) const
{
	const auto it = m_valueMap.find(valueId);
	return it != m_valueMap.end() ? it->second : 0.0f;
}

}
