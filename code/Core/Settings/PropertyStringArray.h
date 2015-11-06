#ifndef traktor_PropertyStringArray_H
#define traktor_PropertyStringArray_H

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

/*! \brief String array property value.
 * \ingroup Core
 */
class T_DLLCLASS PropertyStringArray : public IPropertyValue
{
	T_RTTI_CLASS;

public:
	typedef std::vector< std::wstring > value_type_t;

	PropertyStringArray(const value_type_t& value = value_type_t());

	static value_type_t get(const IPropertyValue* value);

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

protected:
	virtual Ref< IPropertyValue > join(const IPropertyValue* right) const T_OVERRIDE T_FINAL;

	virtual Ref< IPropertyValue > clone() const T_OVERRIDE T_FINAL;

private:
	value_type_t m_value;
};

}

#endif	// traktor_PropertyStringArray_H
