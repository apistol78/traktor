/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Settings/PropertyFloat.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.PropertyFloat", 0, PropertyFloat, IPropertyValue)

PropertyFloat::PropertyFloat(value_type_t value)
:	m_value(value)
{
}

PropertyFloat::value_type_t PropertyFloat::get(const IPropertyValue* value)
{
	return value ? checked_type_cast< const PropertyFloat* >(value)->m_value : 0.0f;
}

void PropertyFloat::serialize(ISerializer& s)
{
	s >> Member< value_type_t >(L"value", m_value);
}

Ref< IPropertyValue > PropertyFloat::join(const IPropertyValue* right) const
{
	return right->clone();
}

Ref< IPropertyValue > PropertyFloat::clone() const
{
	return new PropertyFloat(m_value);
}

}
