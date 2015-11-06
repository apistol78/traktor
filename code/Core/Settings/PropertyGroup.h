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
	 * bool foo = settings->getProperty< PropertyBoolean >("foo", false);
	 */
	template < typename PropertyType >
	typename PropertyType::value_type_t getProperty(const std::wstring& propertyName, typename PropertyType::value_type_t defaultValue) const
	{
		Ref< const IPropertyValue > value = getProperty(propertyName);
		return value ? PropertyType::get(value) : defaultValue;
	}

	template < typename PropertyType >
	typename PropertyType::value_type_t getProperty(const std::wstring& propertyName) const
	{
		Ref< const IPropertyValue > value = getProperty(propertyName);
		return PropertyType::get(value);
	}

	Ref< PropertyGroup > mergeJoin(const PropertyGroup* rightGroup) const;

	Ref< PropertyGroup > mergeReplace(const PropertyGroup* rightGroup) const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const std::map< std::wstring, Ref< IPropertyValue > >& getValues() const { return m_value; }

protected:
	virtual Ref< IPropertyValue > join(const IPropertyValue* rightGroup) const T_OVERRIDE T_FINAL;

	virtual Ref< IPropertyValue > clone() const T_OVERRIDE T_FINAL;

private:
	std::map< std::wstring, Ref< IPropertyValue > > m_value;
};

}

#endif	// traktor_PropertyGroup_H
