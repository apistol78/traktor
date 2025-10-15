/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Settings/PropertyVector.h"

#include "Core/Serialization/ISerializer.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.PropertyVector", 0, PropertyVector, IPropertyValue)

PropertyVector::PropertyVector(const value_type_t& value)
	: m_value(value)
{
}

PropertyVector::value_type_t PropertyVector::get(const IPropertyValue* value)
{
	return value ? checked_type_cast< const PropertyVector* >(value)->m_value : value_type_t();
}

void PropertyVector::serialize(ISerializer& s)
{
	s >> Member< value_type_t >(L"value", m_value);
}

Ref< IPropertyValue > PropertyVector::join(const IPropertyValue* right) const
{
	return right->clone();
}

Ref< IPropertyValue > PropertyVector::clone() const
{
	return new PropertyVector(m_value);
}

}
