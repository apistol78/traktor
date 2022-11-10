/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Xml/Attribute.h"

namespace traktor::xml
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.xml.Attribute", Attribute, Object)

Attribute::Attribute(const std::wstring& name, const std::wstring& value)
:	m_name(name)
,	m_value(value)
,	m_previous(0)
{
}

const std::wstring& Attribute::getName() const
{
	return m_name;
}

void Attribute::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& Attribute::getValue() const
{
	return m_value;
}

void Attribute::setValue(const std::wstring& value)
{
	m_value = value;
}

Attribute* Attribute::getPrevious() const
{
	return m_previous;
}

Attribute* Attribute::getNext() const
{
	return m_next;
}

}
