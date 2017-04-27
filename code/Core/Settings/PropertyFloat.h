/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_PropertyFloat_H
#define traktor_PropertyFloat_H

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

/*! \brief Single precision float property value.
 * \ingroup Core
 */
class T_DLLCLASS PropertyFloat : public IPropertyValue
{
	T_RTTI_CLASS;

public:
	typedef float value_type_t;

	PropertyFloat(value_type_t value = value_type_t());

	static value_type_t get(const IPropertyValue* value);

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	operator value_type_t () const { return m_value; }

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
struct PropertyTrait< float >
{
	typedef PropertyFloat property_type_t;
	typedef float default_value_type_t;
	typedef float return_type_t;
};

}

#endif	// traktor_PropertyFloat_H
