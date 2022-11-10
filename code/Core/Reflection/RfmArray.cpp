/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Reflection/RfmArray.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmArray", RfmArray, RfmCompound)

RfmArray::RfmArray(const wchar_t* name)
:	RfmCompound(name)
{
}

void RfmArray::insertDefault()
{
	m_insertDefaultCount++;
}

}
