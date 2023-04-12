/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Reflection/ReflectionMember.h"
#include "Core/Serialization/Attribute.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ReflectionMember", ReflectionMember, Object)

const wchar_t* ReflectionMember::getName() const
{
	return m_name;
}

const Attribute* ReflectionMember::getAttributes() const
{
	return m_attributes;
}

ReflectionMember::ReflectionMember(const wchar_t* name, const Attribute* attributes)
:	m_name(name)
,	m_attributes(attributes ? attributes->clone() : nullptr)
{
}

}
