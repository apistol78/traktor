/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Settings/PropertyString.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.PropertyString", 0, PropertyString, IPropertyValue)

PropertyString::PropertyString(value_type_t value)
:	m_value(value)
{
}

PropertyString::value_type_t PropertyString::get(const IPropertyValue* value)
{
	return value ? checked_type_cast< const PropertyString* >(value)->m_value : L"";
}

void PropertyString::serialize(ISerializer& s)
{
	s >> Member< value_type_t >(L"value", m_value);
}

Ref< IPropertyValue > PropertyString::join(const IPropertyValue* right) const
{
	return right->clone();
}

Ref< IPropertyValue > PropertyString::clone() const
{
	return new PropertyString(m_value);
}

}
