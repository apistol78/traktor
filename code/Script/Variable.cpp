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
#include "Core/Serialization/MemberRef.h"
#include "Script/IValue.h"
#include "Script/Variable.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.Variable", 0, Variable, ISerializable)

Variable::Variable(const std::wstring& name, const std::wstring& typeName, const IValue* value)
:	m_name(name)
,	m_typeName(typeName)
,	m_value(value)
{
}

void Variable::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& Variable::getName() const
{
	return m_name;
}

void Variable::setTypeName(const std::wstring& typeName)
{
	m_typeName = typeName;
}

const std::wstring& Variable::getTypeName() const
{
	return m_typeName;
}

void Variable::setValue(const IValue* value)
{
	m_value = value;
}

const IValue* Variable::getValue() const
{
	return m_value;
}

void Variable::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< std::wstring >(L"typeName", m_typeName);
	s >> MemberRef< const IValue >(L"value", m_value);
}

	}
}
