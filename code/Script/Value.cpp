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
#include "Script/Value.h"

namespace traktor::script
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.Value", 0, Value, IValue)

Value::Value(const std::wstring& literal)
:	m_literal(literal)
{
}

void Value::setLiteral(const std::wstring& literal)
{
	m_literal = literal;
}

const std::wstring& Value::getLiteral() const
{
	return m_literal;
}

void Value::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"literal", m_literal);
}

}
