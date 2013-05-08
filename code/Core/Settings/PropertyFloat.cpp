#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
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
	return value ? checked_type_cast< const PropertyFloat* >(value)->m_value : false;
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
