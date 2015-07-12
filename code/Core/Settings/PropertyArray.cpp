#include "Core/Serialization/ISerializer.h"
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
	return m_values.size();
}

IPropertyValue* PropertyArray::getProperty(uint32_t index)
{
	if (index < m_values.size())
		return m_values[index];
	else
		return 0;
}

const IPropertyValue* PropertyArray::getProperty(uint32_t index) const
{
	if (index < m_values.size())
		return m_values[index];
	else
		return 0;
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
	else
		return right->clone();
}

Ref< IPropertyValue > PropertyArray::clone() const
{
	Ref< PropertyArray > cloneArray = new PropertyArray();
	for (RefArray< IPropertyValue >::const_iterator i = m_values.begin(); i != m_values.end(); ++i)
	{
		if ((*i))
			cloneArray->addProperty((*i)->clone());
	}
	return cloneArray;
}

}
