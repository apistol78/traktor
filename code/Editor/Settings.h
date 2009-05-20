#ifndef traktor_editor_Settings_H
#define traktor_editor_Settings_H

#include <vector>
#include <map>
#include <string>
#include "Core/Heap/Ref.h"
#include "Core/Guid.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Quaternion.h"
#include "Core/Serialization/Serializable.h"

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

/*! \brief Property value base. */
class T_DLLCLASS PropertyValue : public Serializable
{
	T_RTTI_CLASS(PropertyValue)
};

/*! \brief Boolean property value. */
class T_DLLCLASS PropertyBoolean : public PropertyValue
{
	T_RTTI_CLASS(PropertyBoolean)

public:
	typedef bool value_type_t;

	PropertyBoolean(value_type_t value = false);

	static value_type_t get(const PropertyValue* value);

	virtual bool serialize(Serializer& s);

private:
	value_type_t m_value;
};

/*! \brief Integer property value. */
class T_DLLCLASS PropertyInteger : public PropertyValue
{
	T_RTTI_CLASS(PropertyInteger)

public:
	typedef int32_t value_type_t;

	PropertyInteger(value_type_t value = value_type_t());

	static value_type_t get(const PropertyValue* value);

	virtual bool serialize(Serializer& s);

private:
	value_type_t m_value;
};

/*! \brief Single precision float property value. */
class T_DLLCLASS PropertyFloat : public PropertyValue
{
	T_RTTI_CLASS(PropertyFloat)

public:
	typedef float value_type_t;

	PropertyFloat(value_type_t value = value_type_t());

	static value_type_t get(const PropertyValue* value);

	virtual bool serialize(Serializer& s);

private:
	value_type_t m_value;
};

/*! \brief String property value. */
class T_DLLCLASS PropertyString : public PropertyValue
{
	T_RTTI_CLASS(PropertyBoolean)

public:
	typedef std::wstring value_type_t;

	PropertyString(value_type_t value = L"");

	static value_type_t get(const PropertyValue* value);

	virtual bool serialize(Serializer& s);

private:
	value_type_t m_value;
};

/*! \brief String array property value. */
class T_DLLCLASS PropertyStringArray : public PropertyValue
{
	T_RTTI_CLASS(PropertyStringArray)

public:
	typedef std::vector< std::wstring > value_type_t;

	PropertyStringArray(const value_type_t& value = value_type_t());

	static value_type_t get(const PropertyValue* value);

	virtual bool serialize(Serializer& s);

private:
	value_type_t m_value;
};

/*! \brief Guid property value. */
class T_DLLCLASS PropertyGuid : public PropertyValue
{
	T_RTTI_CLASS(PropertyGuid)

public:
	typedef Guid value_type_t;

	PropertyGuid(const value_type_t& value = value_type_t());

	static value_type_t get(const PropertyValue* value);

	virtual bool serialize(Serializer& s);

private:
	value_type_t m_value;
};

/*! \brief Guid array property value. */
class T_DLLCLASS PropertyGuidArray : public PropertyValue
{
	T_RTTI_CLASS(PropertyGuidArray)

public:
	typedef std::vector< Guid > value_type_t;

	PropertyGuidArray(const value_type_t& value = value_type_t());

	static value_type_t get(const PropertyValue* value);

	virtual bool serialize(Serializer& s);

private:
	value_type_t m_value;
};

/*! \brief Type property value. */
class T_DLLCLASS PropertyType : public PropertyValue
{
	T_RTTI_CLASS(PropertyType)

public:
	typedef const Type* value_type_t;

	PropertyType(value_type_t value = 0);

	static value_type_t get(const PropertyValue* value);

	virtual bool serialize(Serializer& s);

private:
	value_type_t m_value;
};

/*! \brief Type set property value. */
class T_DLLCLASS PropertyTypeSet : public PropertyValue
{
	T_RTTI_CLASS(PropertyTypeSet)

public:
	typedef TypeSet value_type_t;

	PropertyTypeSet(const value_type_t& value = value_type_t());

	static value_type_t get(const PropertyValue* value);

	virtual bool serialize(Serializer& s);

private:
	std::vector< std::wstring > m_value;
};

/*! \brief Vector4 property value. */
class T_DLLCLASS PropertyVector4 : public PropertyValue
{
	T_RTTI_CLASS(PropertyVector4)

public:
	typedef Vector4 value_type_t;

	PropertyVector4(const value_type_t& value = value_type_t::zero());

	static value_type_t get(const PropertyValue* value);

	virtual bool serialize(Serializer& s);

private:
	value_type_t m_value;
};

/*! \brief Quaternion property value. */
class T_DLLCLASS PropertyQuaternion : public PropertyValue
{
	T_RTTI_CLASS(PropertyQuaternion)

public:
	typedef Quaternion value_type_t;

	PropertyQuaternion(const value_type_t& value = value_type_t::identity());

	static value_type_t get(const PropertyValue* value);

	virtual bool serialize(Serializer& s);

private:
	value_type_t m_value;
};

/*! \brief Key property. */
class T_DLLCLASS PropertyKey : public PropertyValue
{
	T_RTTI_CLASS(PropertyKey)

public:
	typedef std::pair< int, int > value_type_t;

	PropertyKey(const value_type_t& value = value_type_t(0, 0));

	static value_type_t get(const PropertyValue* value);

	virtual bool serialize(Serializer& s);

private:
	value_type_t m_value;
};

/*! \brief Property value map. */
class T_DLLCLASS PropertyGroup : public PropertyValue
{
	T_RTTI_CLASS(PropertyGroup)

public:
	typedef std::map< std::wstring, Ref< PropertyValue > > value_type_t;

	PropertyGroup(const value_type_t& value = value_type_t());

	static value_type_t get(const PropertyValue* value);

	void setProperty(const std::wstring& propertyName, PropertyValue* value);

	PropertyValue* getProperty(const std::wstring& propertyName);

	const PropertyValue* getProperty(const std::wstring& propertyName) const;

	/*! \brief Set user property.
	 *
	 * settings->setProperty< PropertyBoolean >("foo", true);
	 */
	template < typename PropertyType >
	void setProperty(const std::wstring& propertyName, typename PropertyType::value_type_t value)
	{
		setProperty(propertyName, gc_new< PropertyType >(value));
	}

	/*! \brief Get user property.
	 *
	 * bool foo = settings->getProperty< PropertyBoolean >("foo", false);
	 */
	template < typename PropertyType >
	typename PropertyType::value_type_t getProperty(const std::wstring& propertyName, typename PropertyType::value_type_t defaultValue) const
	{
		const PropertyValue* value = getProperty(propertyName);
		return value ? PropertyType::get(value) : defaultValue;
	}

	template < typename PropertyType >
	typename PropertyType::value_type_t getProperty(const std::wstring& propertyName) const
	{
		const PropertyValue* value = getProperty(propertyName);
		return PropertyType::get(value);
	}

	virtual bool serialize(Serializer& s);

private:
	value_type_t m_value;
};

/*! \brief Serializable property. */
class T_DLLCLASS PropertySerializable : public PropertyValue
{
	T_RTTI_CLASS(PropertyQuaternion)

public:
	typedef Serializable* value_type_t;

	PropertySerializable(const value_type_t& value = 0);

	static value_type_t get(const PropertyValue* value);

	virtual bool serialize(Serializer& s);

private:
	Ref< Serializable > m_value;
};

class T_DLLCLASS Settings : public Object
{
	T_RTTI_CLASS(Settings)

public:
	/*! \brief Construct settings object.
	 *
	 * The settings concise of two "identical" groups but only
	 * the user group is modifiable, ie. it's saved separatly from
	 * the global group with the current user suffixed to it's filename.
	 *
	 * \param globalGroup Global property group.
	 * \param userGroup User property group.
	 */
	Settings(PropertyGroup* globalGroup, PropertyGroup* userGroup);

	void setProperty(const std::wstring& propertyName, PropertyValue* value);

	PropertyValue* getProperty(const std::wstring& propertyName);

	const PropertyValue* getProperty(const std::wstring& propertyName) const;

	/*! \brief Set user property.
	 *
	 * settings->setProperty< PropertyBoolean >("foo", true);
	 */
	template < typename PropertyType >
	void setProperty(const std::wstring& propertyName, typename const PropertyType::value_type_t& value)
	{
		setProperty(propertyName, gc_new< PropertyType >(cref(value)));
	}

	/*! \brief Get user property.
	 *
	 * bool foo = settings->getProperty< PropertyBoolean >("foo", false);
	 */
	template < typename PropertyType >
	typename PropertyType::value_type_t getProperty(const std::wstring& propertyName, typename const PropertyType::value_type_t& defaultValue) const
	{
		const PropertyValue* value = getProperty(propertyName);
		return value ? PropertyType::get(value) : defaultValue;
	}

	template < typename PropertyType >
	typename PropertyType::value_type_t getProperty(const std::wstring& propertyName) const
	{
		const PropertyValue* value = getProperty(propertyName);
		return PropertyType::get(value);
	}

	inline PropertyGroup* getGlobalGroup() { return m_globalGroup; }

	inline PropertyGroup* getUserGroup() { return m_userGroup; }

private:
	Ref< PropertyGroup > m_globalGroup;
	Ref< PropertyGroup > m_userGroup;
};

	}
}

#endif	// traktor_editor_Settings_H
