#ifndef traktor_Settings_H
#define traktor_Settings_H

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IPropertyValue;
class ISerializer;
class IStream;
class PropertyGroup;

/*! \brief Settings.
 * \ingroup Core
 */
class T_DLLCLASS Settings : public Object
{
	T_RTTI_CLASS;

public:
	Settings();

	/*! \brief
	 */
	static Ref< Settings > read(ISerializer& s);

	/*! \brief
	 */
	bool write(ISerializer& s) const;

	/*! \brief
	 */
	template < typename SerializerType >
	static Ref< Settings > read(IStream* stream)
	{
		SerializerType s(stream);
		return read(s);
	}

	/*! \brief
	 */
	template < typename SerializerType >
	bool write(IStream* stream) const
	{
		SerializerType s(stream);
		return write(s);
	}

	/*! \brief
	 */
	void setProperty(const std::wstring& propertyName, IPropertyValue* value);

	/*! \brief
	 */
	Ref< const IPropertyValue > getProperty(const std::wstring& propertyName) const;

	/*! \brief Set property.
	 *
	 * settings->setProperty< PropertyBoolean >("foo", true);
	 */
	template < typename PropertyType >
	void setProperty(const std::wstring& propertyName, const typename PropertyType::value_type_t& value)
	{
		Ref< PropertyType > property = new PropertyType(value);
		setProperty(propertyName, property);
	}

	/*! \brief Get property.
	 *
	 * bool foo = settings->getProperty< PropertyBoolean >("foo", false);
	 */
	template < typename PropertyType >
	typename PropertyType::value_type_t getProperty(const std::wstring& propertyName, const typename PropertyType::value_type_t& defaultValue) const
	{
		Ref< const IPropertyValue > value = getProperty(propertyName);
		return value ? PropertyType::get(value) : defaultValue;
	}

	/*! \brief
	 */
	template < typename PropertyType >
	typename PropertyType::value_type_t getProperty(const std::wstring& propertyName) const
	{
		Ref< const IPropertyValue > value = getProperty(propertyName);
		return PropertyType::get(value);
	}

	/*! \brief
	 */
	Ref< PropertyGroup > getRootGroup() const { return m_rootGroup; }

private:
	Ref< PropertyGroup > m_rootGroup;
};

}

#endif	// traktor_Settings_H
