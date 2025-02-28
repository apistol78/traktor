/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Settings/PropertyInteger.h"

#include "Core/Serialization/ISerializer.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.PropertyInteger", 0, PropertyInteger, IPropertyValue)

PropertyInteger::PropertyInteger(value_type_t value)
	: m_value(value)
{
}

PropertyInteger::value_type_t PropertyInteger::get(const IPropertyValue* value)
{
	return value ? checked_type_cast< const PropertyInteger* >(value)->m_value : false;
}

void PropertyInteger::serialize(ISerializer& s)
{
	s >> Member< value_type_t >(L"value", m_value);
}

Ref< IPropertyValue > PropertyInteger::join(const IPropertyValue* right) const
{
	return right->clone();
}

Ref< IPropertyValue > PropertyInteger::clone() const
{
	return new PropertyInteger(m_value);
}

}
