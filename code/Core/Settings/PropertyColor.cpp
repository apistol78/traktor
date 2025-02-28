/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Settings/PropertyColor.h"

#include "Core/Serialization/ISerializer.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.PropertyColor", 0, PropertyColor, IPropertyValue)

PropertyColor::PropertyColor(const value_type_t& value)
	: m_value(value)
{
}

PropertyColor::value_type_t PropertyColor::get(const IPropertyValue* value)
{
	return value ? checked_type_cast< const PropertyColor* >(value)->m_value : value_type_t();
}

void PropertyColor::serialize(ISerializer& s)
{
	s >> Member< value_type_t >(L"value", m_value);
}

Ref< IPropertyValue > PropertyColor::join(const IPropertyValue* right) const
{
	return right->clone();
}

Ref< IPropertyValue > PropertyColor::clone() const
{
	return new PropertyColor(m_value);
}

}
