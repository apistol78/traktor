#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Settings/PropertyObject.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLONABLE_CLASS(L"traktor.PropertyObject", 0, PropertyObject, IPropertyValue)

PropertyObject::PropertyObject()
{
}

PropertyObject::PropertyObject(value_type_t value)
:	m_value(value)
{
}

PropertyObject::value_type_t PropertyObject::get(const IPropertyValue* value)
{
	return value ? checked_type_cast< const PropertyObject* >(value)->m_value : value_type_t(0);
}

bool PropertyObject::serialize(ISerializer& s)
{
	return s >> MemberRef< ISerializable >(L"value", m_value);
}

Ref< IPropertyValue > PropertyObject::join(const IPropertyValue* right) const
{
	return clone_instance(right);
}

}
