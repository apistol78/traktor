#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Settings/PropertyBoolean.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.PropertyBoolean", 0, PropertyBoolean, IPropertyValue)

PropertyBoolean::PropertyBoolean(value_type_t value)
:	m_value(value)
{
}

PropertyBoolean::value_type_t PropertyBoolean::get(const IPropertyValue* value)
{
	return value ? checked_type_cast< const PropertyBoolean* >(value)->m_value : false;
}

void PropertyBoolean::serialize(ISerializer& s)
{
	s >> Member< value_type_t >(L"value", m_value);
}

Ref< IPropertyValue > PropertyBoolean::join(const IPropertyValue* right) const
{
	return right->clone();
}

Ref< IPropertyValue > PropertyBoolean::clone() const
{
	return new PropertyBoolean(m_value);
}

}
