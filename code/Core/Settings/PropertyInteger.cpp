/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Settings/PropertyInteger.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.PropertyInteger", 0, PropertyInteger, IPropertyValue)

PropertyInteger::PropertyInteger(value_type_t value)
:	m_value(value)
{
}

PropertyInteger::value_type_t PropertyInteger::get(const IPropertyValue* value)
{
	return value ? checked_type_cast< const PropertyInteger* >(value)->m_value : false;
}

void PropertyInteger::serialize(ISerializer& s)
{
	s >> Member< value_type_t >(L"value", m_value);
}

Ref< IPropertyValue > PropertyInteger::join(const IPropertyValue* right) const
{
	return right->clone();
}

Ref< IPropertyValue > PropertyInteger::clone() const
{
	return new PropertyInteger(m_value);
}

}
