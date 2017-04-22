#ifndef traktor_PropertyInteger_H
#define traktor_PropertyInteger_H

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

/*! \brief Integer property value.
 * \ingroup Core
 */
class T_DLLCLASS PropertyInteger : public IPropertyValue
{
	T_RTTI_CLASS;

public:
	typedef int32_t value_type_t;

	PropertyInteger(value_type_t value = value_type_t());

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
struct PropertyTrait< int32_t >
{
	typedef PropertyInteger property_type_t;
	typedef int32_t default_value_type_t;
	typedef int32_t return_type_t;
};

}

#endif	// traktor_PropertyInteger_H
