/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_PropertyGroup_H
#define traktor_PropertyGroup_H

#include <map>
#include "Core/Settings/IPropertyValue.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	
/*! \brief Property value map.
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

	PropertyGroup();

	PropertyGroup(const std::map< std::wstring, Ref< IPropertyValue > >& value);

	static value_type_t get(const IPropertyValue* value);

	void setProperty(const std::wstring& propertyName, IPropertyValue* value);

	IPropertyValue* getProperty(const std::wstring& propertyName);

	const IPropertyValue* getProperty(const std::wstring& propertyName) const;

	/*! \brief Set user property.
	 *
	 * settings->setProperty< PropertyBoolean >("foo", true);
	 */
	template < typename PropertyType >
	void setProperty(const std::wstring& propertyName, typename PropertyType::value_type_t value)
	{
		Ref< PropertyType > property = new PropertyType(value);
		setProperty(propertyName, property);
	}

	/*! \brief Get user property.
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

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const std::map< std::wstring, Ref< IPropertyValue > >& getValues() const { return m_value; }

protected:
	virtual Ref< IPropertyValue > join(const IPropertyValue* rightGroup) const T_OVERRIDE T_FINAL;

	virtual Ref< IPropertyValue > clone() const T_OVERRIDE T_FINAL;

private:
	std::map< std::wstring, Ref< IPropertyValue > > m_value;
};

/*!
 * \ingroup Core
 */
template< >
struct PropertyTrait< PropertyGroup >
{
	typedef PropertyGroup property_type_t;
	typedef PropertyGroup* default_value_type_t;
	typedef Ref< PropertyGroup > return_type_t;
};

/*!
 * \ingroup Core
 */
template< >
struct PropertyTrait< Ref< PropertyGroup > >
{
	typedef PropertyGroup property_type_t;
	typedef PropertyGroup* default_value_type_t;
	typedef Ref< PropertyGroup > return_type_t;
};

}

#endif	// traktor_PropertyGroup_H
