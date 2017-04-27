/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_PropertyStringSet_H
#define traktor_PropertyStringSet_H

#include <set>
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

/*! \brief String set property value.
 * \ingroup Core
 */
class T_DLLCLASS PropertyStringSet : public IPropertyValue
{
	T_RTTI_CLASS;

public:
	typedef std::set< std::wstring > value_type_t;

	PropertyStringSet(const value_type_t& value = value_type_t());

	static value_type_t get(const IPropertyValue* value);

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

protected:
	virtual Ref< IPropertyValue > join(const IPropertyValue* right) const T_OVERRIDE T_FINAL;

	virtual Ref< IPropertyValue > clone() const T_OVERRIDE T_FINAL;

private:
	value_type_t m_value;
};

/*!
 * \ingroup Core
 */
template< >
struct PropertyTrait< std::set< std::wstring > >
{
	typedef PropertyStringSet property_type_t;
	typedef const std::set< std::wstring >& default_value_type_t;
	typedef std::set< std::wstring > return_type_t;
};

}

#endif	// traktor_PropertyStringSet_H
