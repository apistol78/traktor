/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Settings/PropertyStringArray.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.PropertyStringArray", 0, PropertyStringArray, IPropertyValue)

PropertyStringArray::PropertyStringArray(const value_type_t& value)
:	m_value(value)
{
}

PropertyStringArray::value_type_t PropertyStringArray::get(const IPropertyValue* value)
{
	return value ? checked_type_cast< const PropertyStringArray* >(value)->m_value : value_type_t();
}

void PropertyStringArray::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< std::wstring >(L"value", m_value);
}

Ref< IPropertyValue > PropertyStringArray::join(const IPropertyValue* right) const
{
	if (const PropertyStringArray* rightStringArray = dynamic_type_cast< const PropertyStringArray* >(right))
	{
		Ref< PropertyStringArray > leftStringArray = new PropertyStringArray(m_value);
		leftStringArray->m_value.insert(
			leftStringArray->m_value.end(),
			rightStringArray->m_value.begin(),
			rightStringArray->m_value.end()
		);
		return leftStringArray;
	}
	else
		return right->clone();
}

Ref< IPropertyValue > PropertyStringArray::clone() const
{
	return new PropertyStringArray(m_value);
}

}
