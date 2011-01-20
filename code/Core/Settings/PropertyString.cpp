#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
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

IPropertyValue* PropertyString::merge(IPropertyValue* right, bool join)
{
	return right;
}

bool PropertyString::serialize(ISerializer& s)
{
	return s >> Member< value_type_t >(L"value", m_value);
}

}
