#ifndef traktor_PropertyArray_H
#define traktor_PropertyArray_H

#include "Core/RefArray.h"
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
	
/*! \brief Property value array.
 * \ingroup Core
 */
class T_DLLCLASS PropertyArray : public IPropertyValue
{
	T_RTTI_CLASS;

public:
	typedef RefArray< IPropertyValue > value_type_t;

	PropertyArray();

	PropertyArray(const RefArray< IPropertyValue >& values);

	static value_type_t get(const IPropertyValue* value);

	void addProperty(IPropertyValue* value);

	void removeProperty(IPropertyValue* value);

	uint32_t getPropertyCount() const;

	IPropertyValue* getProperty(uint32_t index);

	const IPropertyValue* getProperty(uint32_t index) const;

	template < typename PropertyType >
	typename PropertyType::value_type_t getProperty(uint32_t index, typename PropertyType::value_type_t defaultValue) const
	{
		Ref< const IPropertyValue > value = getProperty(index);
		return value ? PropertyType::get(value) : defaultValue;
	}

	template < typename PropertyType >
	typename PropertyType::value_type_t getProperty(uint32_t index) const
	{
		Ref< const IPropertyValue > value = getProperty(index);
		return PropertyType::get(value);
	}

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const RefArray< IPropertyValue >& getValues() const { return m_values; }

protected:
	virtual Ref< IPropertyValue > join(const IPropertyValue* rightGroup) const T_OVERRIDE T_FINAL;

	virtual Ref< IPropertyValue > clone() const T_OVERRIDE T_FINAL;

private:
	RefArray< IPropertyValue > m_values;
};

}

#endif	// traktor_PropertyArray_H
