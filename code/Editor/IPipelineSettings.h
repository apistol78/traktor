#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Settings/IPropertyValue.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::editor
{

class T_DLLCLASS IPipelineSettings : public Object
{
	T_RTTI_CLASS;

public:
	/*! Get property from settings.
	 * 
	 * \param propertyName Name of property.
	 * \param includeInHash If property value should be part of build hash, only properties which affect output result should be part of hash.
	 * \param defaultValue Default value as a property value, necessary to keep hash integrity in case no property was found.
	 * \return Property value.
	 */
	virtual Ref< const IPropertyValue > getProperty(const std::wstring& propertyName, bool includeInHash, const IPropertyValue* defaultValue) const = 0;

	template < typename ValueType >
	typename PropertyTrait< ValueType >::return_type_t getPropertyExcludeHash(const std::wstring& propertyName, typename PropertyTrait< ValueType >::default_value_type_t defaultValue) const
	{
		Ref< const IPropertyValue > value = getProperty(propertyName, false, nullptr);
		return value ? PropertyTrait< ValueType >::property_type_t::get(value) : defaultValue;
	}

	template < typename ValueType >
	typename PropertyTrait< ValueType >::return_type_t getPropertyExcludeHash(const std::wstring& propertyName) const
	{
		Ref< const IPropertyValue > value = getProperty(propertyName, false, nullptr);
		return PropertyTrait< ValueType >::property_type_t::get(value);
	}

	template < typename ValueType >
	typename PropertyTrait< ValueType >::return_type_t getPropertyIncludeHash(const std::wstring& propertyName, typename PropertyTrait< ValueType >::default_value_type_t defaultValue) const
	{
		const typename PropertyTrait< ValueType >::property_type_t hashableDefaultValue(defaultValue);
		Ref< const IPropertyValue > value = getProperty(propertyName, true, &hashableDefaultValue);
		return PropertyTrait< ValueType >::property_type_t::get(value);
	}

	template < typename ValueType >
	typename PropertyTrait< ValueType >::return_type_t getPropertyIncludeHash(const std::wstring& propertyName) const
	{
		const typename PropertyTrait< ValueType >::property_type_t hashableDefaultValue;
		Ref< const IPropertyValue > value = getProperty(propertyName, true, &hashableDefaultValue);
		return PropertyTrait< ValueType >::property_type_t::get(value);
	}
};

}
