/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Settings/PropertyArray.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.PropertyArray", 0, PropertyArray, IPropertyValue)

PropertyArray::PropertyArray()
{
}

PropertyArray::PropertyArray(const RefArray< IPropertyValue >& values)
:	m_values(values)
{
}

PropertyArray::value_type_t PropertyArray::get(const IPropertyValue* value)
{
	return value ? static_cast< const PropertyArray* >(value)->m_values : value_type_t();
}

void PropertyArray::addProperty(IPropertyValue* value)
{
	m_values.push_back(value);
}

void PropertyArray::removeProperty(IPropertyValue* value)
{
	m_values.remove(value);
}

uint32_t PropertyArray::getPropertyCount() const
{
	return (uint32_t)m_values.size();
}

IPropertyValue* PropertyArray::getProperty(uint32_t index)
{
	if (index < m_values.size())
		return m_values[index];
	else
		return nullptr;
}

const IPropertyValue* PropertyArray::getProperty(uint32_t index) const
{
	if (index < m_values.size())
		return m_values[index];
	else
		return nullptr;
}

void PropertyArray::serialize(ISerializer& s)
{
	s >> MemberRefArray< IPropertyValue >(L"values", m_values);
}

Ref< IPropertyValue > PropertyArray::join(const IPropertyValue* right) const
{
	if (const PropertyArray* rightArray = dynamic_type_cast< const PropertyArray* >(right))
	{
		const RefArray< IPropertyValue >& leftValues = getValues();
		const RefArray< IPropertyValue >& rightValues = rightArray->getValues();

		Ref< PropertyArray > joinedArray = new PropertyArray();

		joinedArray->m_values.insert(
			joinedArray->m_values.end(),
			leftValues.begin(),
			leftValues.end()
		);
		joinedArray->m_values.insert(
			joinedArray->m_values.end(),
			rightValues.begin(),
			rightValues.end()
		);

		return joinedArray;
	}
	else if (right)
		return right->clone();
	else
		return nullptr;
}

Ref< IPropertyValue > PropertyArray::clone() const
{
	Ref< PropertyArray > cloneArray = new PropertyArray();
	for (auto value : m_values)
	{
		if (value)
			cloneArray->addProperty(value->clone());
	}
	return cloneArray;
}

}
