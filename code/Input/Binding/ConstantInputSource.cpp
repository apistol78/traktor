/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/String.h"
#include "Input/Binding/ConstantInputSource.h"

namespace traktor::input
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.ConstantInputSource", ConstantInputSource, IInputSource)

ConstantInputSource::ConstantInputSource(float value)
:	m_value(value)
{
}

std::wstring ConstantInputSource::getDescription() const
{
	return toString(m_value);
}

void ConstantInputSource::prepare(float T, float dT)
{
}

float ConstantInputSource::read(float T, float dT)
{
	return m_value;
}

}
