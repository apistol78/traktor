/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Settings/PropertyStringSet.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.PropertyStringSet", 0, PropertyStringSet, IPropertyValue)

PropertyStringSet::PropertyStringSet(const value_type_t& value)
:	m_value(value)
{
}

PropertyStringSet::value_type_t PropertyStringSet::get(const IPropertyValue* value)
{
	return value ? checked_type_cast< const PropertyStringSet* >(value)->m_value : value_type_t();
}

void PropertyStringSet::serialize(ISerializer& s)
{
	s >> MemberStlSet< std::wstring >(L"value", m_value);
}

Ref< IPropertyValue > PropertyStringSet::join(const IPropertyValue* right) const
{
	if (const PropertyStringSet* rightStringSet = dynamic_type_cast< const PropertyStringSet* >(right))
	{
		Ref< PropertyStringSet > leftStringSet = new PropertyStringSet(m_value);
		leftStringSet->m_value.insert(rightStringSet->m_value.begin(), rightStringSet->m_value.end());
		return leftStringSet;
	}
	else
		return right->clone();
}

Ref< IPropertyValue > PropertyStringSet::clone() const
{
	return new PropertyStringSet(m_value);
}

}
