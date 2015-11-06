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

}

#endif	// traktor_PropertyStringSet_H
