/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Settings/IPropertyValue.h"

#include <string>

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
		const typename PropertyTrait< ValueType >::property_type_t hashableDefaultValue = typename PropertyTrait< ValueType >::property_type_t();
		Ref< const IPropertyValue > value = getProperty(propertyName, true, &hashableDefaultValue);
		return PropertyTrait< ValueType >::property_type_t::get(value);
	}
};

}
