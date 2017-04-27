/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Settings/PropertyColor.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.PropertyColor", 0, PropertyColor, IPropertyValue)

PropertyColor::PropertyColor(const value_type_t& value)
:	m_value(value)
{
}

PropertyColor::value_type_t PropertyColor::get(const IPropertyValue* value)
{
	return value ? checked_type_cast< const PropertyColor* >(value)->m_value : value_type_t();
}

void PropertyColor::serialize(ISerializer& s)
{
	s >> Member< value_type_t >(L"value", m_value);
}

Ref< IPropertyValue > PropertyColor::join(const IPropertyValue* right) const
{
	return right->clone();
}

Ref< IPropertyValue > PropertyColor::clone() const
{
	return new PropertyColor(m_value);
}

}
