#ifndef traktor_PropertyBoolean_H
#define traktor_PropertyBoolean_H

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

/*! \brief Boolean property value.
 * \ingroup Core
 */
class T_DLLCLASS PropertyBoolean : public IPropertyValue
{
	T_RTTI_CLASS;

public:
	typedef bool value_type_t;

	PropertyBoolean(value_type_t value = false);

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

#endif	// traktor_PropertyBoolean_H
