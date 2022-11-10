/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Reflection/RfmObject.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmObject", RfmObject, ReflectionMember)

RfmObject::RfmObject(const wchar_t* name, ISerializable* value)
:	ReflectionMember(name)
,	m_value(value)
{
}

bool RfmObject::replace(const ReflectionMember* source)
{
	if (const RfmObject* sourceObject = dynamic_type_cast< const RfmObject* >(source))
	{
		m_value = sourceObject->m_value;
		return true;
	}
	else
		return false;
}

}
