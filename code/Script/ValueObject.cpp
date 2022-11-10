/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Script/ValueObject.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ValueObject", 0, ValueObject, IValue)

ValueObject::ValueObject(uint32_t objectRef)
:	m_objectRef(objectRef)
{
}

ValueObject::ValueObject(uint32_t objectRef, const std::wstring& valueOf)
:	m_objectRef(objectRef)
,	m_valueOf(valueOf)
{
}

void ValueObject::setObjectRef(uint32_t objectRef)
{
	m_objectRef = objectRef;
}

uint32_t ValueObject::getObjectRef() const
{
	return m_objectRef;
}

void ValueObject::setValueOf(const std::wstring& valueOf)
{
	m_valueOf = valueOf;
}

const std::wstring& ValueObject::getValueOf() const
{
	return m_valueOf;
}

void ValueObject::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"objectRef", m_objectRef);
	s >> Member< std::wstring >(L"valueOf", m_valueOf);
}

	}
}
