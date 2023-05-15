/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Jungle/State/IValue.h"
#include "Jungle/State/State.h"

namespace traktor::jungle
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.jungle.State", State, Object)

State::State(const RefArray< const IValue >& values)
:	m_values(values)
,	m_index(0)
{
}

void State::packBegin()
{
	m_values.resize(0);
}

void State::pack(const IValue* value)
{
	m_values.push_back(value);
}

void State::unpackBegin()
{
	m_index = 0;
}

const IValue* State::unpack()
{
	T_ASSERT(m_index < m_values.size());
	return m_values[m_index++];
}

}
