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

	virtual void serialize(ISerializer& s);

	operator value_type_t () const { return m_value; }

protected:
	virtual Ref< IPropertyValue > join(const IPropertyValue* right) const;

	virtual Ref< IPropertyValue > clone() const;

private:
	value_type_t m_value;
};

}

#endif	// traktor_PropertyFloat_H
