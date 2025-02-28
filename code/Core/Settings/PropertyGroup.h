/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Settings/IPropertyValue.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Property value map.
 * \ingroup Core
 */
class T_DLLCLASS PropertyGroup : public IPropertyValue
{
	T_RTTI_CLASS;

public:
	enum MergeMode
	{
		MmReplace,
		MmJoin
	};

	typedef Ref< PropertyGroup > value_type_t;

	PropertyGroup() = default;

	explicit PropertyGroup(const SmallMap< std::wstring, Ref< IPropertyValue > >& value);

	static value_type_t get(const IPropertyValue* value);

	void setProperty(const std::wstring& propertyName, IPropertyValue* value);

	IPropertyValue* getProperty(const std::wstring& propertyName);

	const IPropertyValue* getProperty(const std::wstring& propertyName) const;

	/*! Set user property.
	 *
	 * settings->setProperty< PropertyBoolean >("foo", true);
	 */
	template < typename PropertyType >
	void setProperty(const std::wstring& propertyName, typename PropertyType::value_type_t value)
	{
		Ref< PropertyType > property = new PropertyType(value);
		setProperty(propertyName, property);
	}

	/*! Get user property.
	 *
	 * bool foo = settings->getProperty< bool >("foo", false);
	 */
	template < typename ValueType >
	typename PropertyTrait< ValueType >::return_type_t getProperty(const std::wstring& propertyName, typename PropertyTrait< ValueType >::default_value_type_t defaultValue) const
	{
		Ref< const IPropertyValue > value = getProperty(propertyName);
		return value ? PropertyTrait< ValueType >::property_type_t::get(value) : defaultValue;
	}

	template < typename ValueType >
	typename PropertyTrait< ValueType >::return_type_t getProperty(const std::wstring& propertyName) const
	{
		Ref< const IPropertyValue > value = getProperty(propertyName);
		return PropertyTrait< ValueType >::property_type_t::get(value);
	}

	Ref< PropertyGroup > merge(const PropertyGroup* rightGroup, MergeMode mode) const;

	Ref< PropertyGroup > difference(const PropertyGroup* rightGroup) const;

	virtual void serialize(ISerializer& s) override;

	const SmallMap< std::wstring, Ref< IPropertyValue > >& getValues() const { return m_value; }

protected:
	virtual Ref< IPropertyValue > join(const IPropertyValue* rightGroup) const override final;

	virtual Ref< IPropertyValue > clone() const override final;

private:
	SmallMap< std::wstring, Ref< IPropertyValue > > m_value;
};

/*!
 * \ingroup Core
 */
template <>
struct PropertyTrait< PropertyGroup >
{
	typedef PropertyGroup property_type_t;
	typedef PropertyGroup* default_value_type_t;
	typedef Ref< PropertyGroup > return_type_t;
};

/*!
 * \ingroup Core
 */
template <>
struct PropertyTrait< Ref< PropertyGroup > >
{
	typedef PropertyGroup property_type_t;
	typedef PropertyGroup* default_value_type_t;
	typedef Ref< PropertyGroup > return_type_t;
};

}
