#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Settings/PropertyStringSet.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLONABLE_CLASS(L"traktor.PropertyStringSet", 0, PropertyStringSet, IPropertyValue)

PropertyStringSet::PropertyStringSet(const value_type_t& value)
:	m_value(value)
{
}

PropertyStringSet::value_type_t PropertyStringSet::get(const IPropertyValue* value)
{
	return value ? checked_type_cast< const PropertyStringSet* >(value)->m_value : value_type_t();
}

bool PropertyStringSet::serialize(ISerializer& s)
{
	return s >> MemberStlSet< std::wstring >(L"value", m_value);
}

Ref< IPropertyValue > PropertyStringSet::join(const IPropertyValue* right) const
{
	if (const PropertyStringSet* rightStringSet = dynamic_type_cast< const PropertyStringSet* >(right))
	{
		Ref< PropertyStringSet > leftStringSet = clone_instance(this);
		leftStringSet->m_value.insert(rightStringSet->m_value.begin(), rightStringSet->m_value.end());
		return leftStringSet;
	}
	else
		return clone_instance(right);
}

}
