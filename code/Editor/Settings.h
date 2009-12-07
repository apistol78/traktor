#ifndef traktor_editor_Settings_H
#define traktor_editor_Settings_H

#include <map>
#include <string>
#include <vector>
#include "Core/Guid.h"
#include "Core/Math/Color.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Quaternion.h"
#include "Core/Serialization/ISerializable.h"
#include "Ui/Enums.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

/*! \brief Property value base.
 * \ingroup Editor
 */
class T_DLLCLASS PropertyValue : public ISerializable
{
	T_RTTI_CLASS;
};

/*! \brief Boolean property value.
 * \ingroup Editor
 */
class T_DLLCLASS PropertyBoolean : public PropertyValue
{
	T_RTTI_CLASS;

public:
	typedef bool value_type_t;

	PropertyBoolean(value_type_t value = false);

	static value_type_t get(PropertyValue* value);

	virtual bool serialize(ISerializer& s);

private:
	value_type_t m_value;
};

/*! \brief Integer property value.
 * \ingroup Editor
 */
class T_DLLCLASS PropertyInteger : public PropertyValue
{
	T_RTTI_CLASS;

public:
	typedef int32_t value_type_t;

	PropertyInteger(value_type_t value = value_type_t());

	static value_type_t get(PropertyValue* value);

	virtual bool serialize(ISerializer& s);

private:
	value_type_t m_value;
};

/*! \brief Single precision float property value.
 * \ingroup Editor
 */
class T_DLLCLASS PropertyFloat : public PropertyValue
{
	T_RTTI_CLASS;

public:
	typedef float value_type_t;

	PropertyFloat(value_type_t value = value_type_t());

	static value_type_t get(PropertyValue* value);

	virtual bool serialize(ISerializer& s);

private:
	value_type_t m_value;
};

/*! \brief String property value.
 * \ingroup Editor
 */
class T_DLLCLASS PropertyString : public PropertyValue
{
	T_RTTI_CLASS;

public:
	typedef std::wstring value_type_t;

	PropertyString(value_type_t value = L"");

	static value_type_t get(PropertyValue* value);

	virtual bool serialize(ISerializer& s);

private:
	value_type_t m_value;
};

/*! \brief String array property value.
 * \ingroup Editor
 */
class T_DLLCLASS PropertyStringArray : public PropertyValue
{
	T_RTTI_CLASS;

public:
	typedef std::vector< std::wstring > value_type_t;

	PropertyStringArray(const value_type_t& value = value_type_t());

	static value_type_t get(PropertyValue* value);

	virtual bool serialize(ISerializer& s);

private:
	value_type_t m_value;
};

/*! \brief Guid property value.
 * \ingroup Editor
 */
class T_DLLCLASS PropertyGuid : public PropertyValue
{
	T_RTTI_CLASS;

public:
	typedef Guid value_type_t;

	PropertyGuid(const value_type_t& value = value_type_t());

	static value_type_t get(PropertyValue* value);

	virtual bool serialize(ISerializer& s);

private:
	value_type_t m_value;
};

/*! \brief Guid array property value.
 * \ingroup Editor
 */
class T_DLLCLASS PropertyGuidArray : public PropertyValue
{
	T_RTTI_CLASS;

public:
	typedef std::vector< Guid > value_type_t;

	PropertyGuidArray(const value_type_t& value = value_type_t());

	static value_type_t get(PropertyValue* value);

	virtual bool serialize(ISerializer& s);

private:
	value_type_t m_value;
};

/*! \brief Type property value.
 * \ingroup Editor
 */
class T_DLLCLASS PropertyType : public PropertyValue
{
	T_RTTI_CLASS;

public:
	typedef const TypeInfo* value_type_t;

	PropertyType(value_type_t value = 0);

	static value_type_t get(PropertyValue* value);

	virtual bool serialize(ISerializer& s);

private:
	value_type_t m_value;
};

/*! \brief Type set property value.
 * \ingroup Editor
 */
class T_DLLCLASS PropertyTypeSet : public PropertyValue
{
	T_RTTI_CLASS;

public:
	typedef TypeInfoSet value_type_t;

	PropertyTypeSet(const value_type_t& value = value_type_t());

	static value_type_t get(PropertyValue* value);

	virtual bool serialize(ISerializer& s);

private:
	std::vector< std::wstring > m_value;
};

/*! \brief Color property value.
 * \ingroup Editor
 */
class T_DLLCLASS PropertyColor : public PropertyValue
{
	T_RTTI_CLASS;

public:
	typedef Color value_type_t;

	PropertyColor(const value_type_t& value = value_type_t());

	static value_type_t get(PropertyValue* value);

	virtual bool serialize(ISerializer& s);

private:
	value_type_t m_value;
};

/*! \brief Vector4 property value.
 * \ingroup Editor
 */
class T_DLLCLASS PropertyVector4 : public PropertyValue
{
	T_RTTI_CLASS;

public:
	typedef Vector4 value_type_t;

	PropertyVector4(const value_type_t& value = value_type_t::zero());

	static value_type_t get(PropertyValue* value);

	virtual bool serialize(ISerializer& s);

private:
	value_type_t m_value;
};

/*! \brief Quaternion property value.
 * \ingroup Editor
 */
class T_DLLCLASS PropertyQuaternion : public PropertyValue
{
	T_RTTI_CLASS;

public:
	typedef Quaternion value_type_t;

	PropertyQuaternion(const value_type_t& value = value_type_t::identity());

	static value_type_t get(PropertyValue* value);

	virtual bool serialize(ISerializer& s);

private:
	value_type_t m_value;
};

/*! \brief Key property.
 * \ingroup Editor
 */
class T_DLLCLASS PropertyKey : public PropertyValue
{
	T_RTTI_CLASS;

public:
	typedef std::pair< int, ui::VirtualKey > value_type_t;

	PropertyKey(const value_type_t& value = value_type_t(0, ui::VkNull));

	static value_type_t get(PropertyValue* value);

	virtual bool serialize(ISerializer& s);

private:
	value_type_t m_value;
};

/*! \brief Property value map.
 * \ingroup Editor
 */
class T_DLLCLASS PropertyGroup : public PropertyValue
{
	T_RTTI_CLASS;

public:
	typedef Ref< PropertyGroup > value_type_t;

	PropertyGroup();

	static value_type_t get(PropertyValue* value);

	void setProperty(const std::wstring& propertyName, PropertyValue* value);

	Ref< PropertyValue > getProperty(const std::wstring& propertyName) const;

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
		Ref< PropertyValue > value = getProperty(propertyName);
		return value ? PropertyType::get(value) : defaultValue;
	}

	template < typename PropertyType >
	typename PropertyType::value_type_t getProperty(const std::wstring& propertyName) const
	{
		Ref< PropertyValue > value = getProperty(propertyName);
		return PropertyType::get(value);
	}

	virtual bool serialize(ISerializer& s);

	const std::map< std::wstring, Ref< PropertyValue > >& getValues() const { return m_value; }

private:
	std::map< std::wstring, Ref< PropertyValue > > m_value;
};

/*! \brief ISerializable property.
 * \ingroup Editor
 */
class T_DLLCLASS PropertySerializable : public PropertyValue
{
	T_RTTI_CLASS;

public:
	typedef Ref< ISerializable > value_type_t;

	PropertySerializable(const value_type_t& value = 0);

	static value_type_t get(PropertyValue* value);

	virtual bool serialize(ISerializer& s);

private:
	Ref< ISerializable > m_value;
};

/*! \brief Settings.
 * \ingroup Editor
 */
class T_DLLCLASS Settings : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Construct settings object.
	 *
	 * The settings concise of two "identical" groups but only
	 * the user group is modifiable, i.e. it's saved separately from
	 * the global group with the current user suffixed to it's filename.
	 *
	 * \param globalGroup Global property group.
	 * \param userGroup User property group.
	 */
	Settings(PropertyGroup* globalGroup, PropertyGroup* userGroup);

	void setProperty(const std::wstring& propertyName, PropertyValue* value);

	Ref< PropertyValue > getProperty(const std::wstring& propertyName) const;

	/*! \brief Set user property.
	 *
	 * settings->setProperty< PropertyBoolean >("foo", true);
	 */
	template < typename PropertyType >
	void setProperty(const std::wstring& propertyName, const typename PropertyType::value_type_t& value)
	{
		Ref< PropertyType > property = new PropertyType(value);
		setProperty(propertyName, property);
	}

	/*! \brief Get user property.
	 *
	 * bool foo = settings->getProperty< PropertyBoolean >("foo", false);
	 */
	template < typename PropertyType >
	typename PropertyType::value_type_t getProperty(const std::wstring& propertyName, const typename PropertyType::value_type_t& defaultValue) const
	{
		Ref< PropertyValue > value = getProperty(propertyName);
		return value ? PropertyType::get(value) : defaultValue;
	}

	template < typename PropertyType >
	typename PropertyType::value_type_t getProperty(const std::wstring& propertyName) const
	{
		Ref< PropertyValue > value = getProperty(propertyName);
		return PropertyType::get(value);
	}

	inline Ref< PropertyGroup > getGlobalGroup() { return m_globalGroup; }

	inline Ref< PropertyGroup > getUserGroup() { return m_userGroup; }

private:
	Ref< PropertyGroup > m_globalGroup;
	Ref< PropertyGroup > m_userGroup;
};

	}
}

#endif	// traktor_editor_Settings_H
