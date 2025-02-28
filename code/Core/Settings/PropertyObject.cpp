/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Settings/PropertyObject.h"

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.PropertyObject", 0, PropertyObject, IPropertyValue)

PropertyObject::PropertyObject(value_type_t value)
	: m_value(value)
{
}

PropertyObject::value_type_t PropertyObject::get(const IPropertyValue* value)
{
	return value ? checked_type_cast< const PropertyObject* >(value)->m_value : nullptr;
}

void PropertyObject::serialize(ISerializer& s)
{
	s >> MemberRef< ISerializable >(L"value", m_value);
}

Ref< IPropertyValue > PropertyObject::join(const IPropertyValue* right) const
{
	return right->clone();
}

Ref< IPropertyValue > PropertyObject::clone() const
{
	return new PropertyObject(m_value);
}

}
