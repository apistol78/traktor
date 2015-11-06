#ifndef traktor_PropertyObject_H
#define traktor_PropertyObject_H

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

/*! \brief Object property value.
 * \ingroup Core
 */
class T_DLLCLASS PropertyObject : public IPropertyValue
{
	T_RTTI_CLASS;

public:
	typedef Ref< ISerializable > value_type_t;

	PropertyObject();

	PropertyObject(value_type_t value);

	static value_type_t get(const IPropertyValue* value);

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

protected:
	virtual Ref< IPropertyValue > join(const IPropertyValue* right) const T_OVERRIDE T_FINAL;

	virtual Ref< IPropertyValue > clone() const T_OVERRIDE T_FINAL;

private:
	value_type_t m_value;
};

}

#endif	// traktor_PropertyObject_H
