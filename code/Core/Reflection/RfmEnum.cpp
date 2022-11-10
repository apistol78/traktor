/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Reflection/RfmEnum.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmEnum", RfmEnum, ReflectionMember)

RfmEnum::RfmEnum(const wchar_t* name, const std::wstring& value)
:	ReflectionMember(name)
,	m_value(value)
{
}

bool RfmEnum::replace(const ReflectionMember* source)
{
	if (const RfmEnum* sourceEnum = dynamic_type_cast< const RfmEnum* >(source))
	{
		m_value = sourceEnum->m_value;
		return true;
	}
	else
		return false;
}

}
