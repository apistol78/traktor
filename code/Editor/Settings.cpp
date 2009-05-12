#include "Editor/Settings.h"
#include "Ui/Application.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberType.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Misc/StringUtils.h"
#include "Core/Misc/SplitString.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PropertyValue", PropertyValue, Serializable)

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.editor.PropertyBoolean", PropertyBoolean, PropertyValue)

PropertyBoolean::PropertyBoolean(value_type_t value)
:	m_value(value)
{
}

PropertyBoolean::value_type_t PropertyBoolean::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertyBoolean* >(value)->m_value : false;
}

bool PropertyBoolean::serialize(Serializer& s)
{
	return s >> Member< value_type_t >(L"value", m_value);
}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.editor.PropertyInteger", PropertyInteger, PropertyValue)

PropertyInteger::PropertyInteger(value_type_t value)
:	m_value(value)
{
}

PropertyInteger::value_type_t PropertyInteger::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertyInteger* >(value)->m_value : false;
}

bool PropertyInteger::serialize(Serializer& s)
{
	return s >> Member< value_type_t >(L"value", m_value);
}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.editor.PropertyFloat", PropertyFloat, PropertyValue)

PropertyFloat::PropertyFloat(value_type_t value)
:	m_value(value)
{
}

PropertyFloat::value_type_t PropertyFloat::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertyFloat* >(value)->m_value : false;
}

bool PropertyFloat::serialize(Serializer& s)
{
	return s >> Member< value_type_t >(L"value", m_value);
}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.editor.PropertyString", PropertyString, PropertyValue)

PropertyString::PropertyString(value_type_t value)
:	m_value(value)
{
}

PropertyString::value_type_t PropertyString::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertyString* >(value)->m_value : L"";
}

bool PropertyString::serialize(Serializer& s)
{
	return s >> Member< value_type_t >(L"value", m_value);
}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.editor.PropertyStringArray", PropertyStringArray, PropertyValue)

PropertyStringArray::PropertyStringArray(const value_type_t& value)
:	m_value(value)
{
}

PropertyStringArray::value_type_t PropertyStringArray::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertyStringArray* >(value)->m_value : value_type_t();
}

bool PropertyStringArray::serialize(Serializer& s)
{
	return s >> MemberStlVector< std::wstring >(L"value", m_value);
}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.editor.PropertyGuid", PropertyGuid, PropertyValue)

PropertyGuid::PropertyGuid(const value_type_t& value)
:	m_value(value)
{
}

PropertyGuid::value_type_t PropertyGuid::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertyGuid* >(value)->m_value : value_type_t();
}

bool PropertyGuid::serialize(Serializer& s)
{
	return s >> Member< Guid >(L"value", m_value);
}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.editor.PropertyGuidArray", PropertyGuidArray, PropertyValue)

PropertyGuidArray::PropertyGuidArray(const value_type_t& value)
:	m_value(value)
{
}

PropertyGuidArray::value_type_t PropertyGuidArray::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertyGuidArray* >(value)->m_value : value_type_t();
}

bool PropertyGuidArray::serialize(Serializer& s)
{
	return s >> MemberStlVector< Guid >(L"value", m_value);
}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.editor.PropertyType", PropertyType, PropertyValue)

PropertyType::PropertyType(value_type_t value)
:	m_value(value)
{
}

PropertyType::value_type_t PropertyType::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertyType* >(value)->m_value : 0;
}

bool PropertyType::serialize(Serializer& s)
{
	return s >> MemberType(L"value", m_value);
}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.editor.PropertyTypeSet", PropertyTypeSet, PropertyValue)

PropertyTypeSet::PropertyTypeSet(const value_type_t& value)
{
	for (value_type_t::const_iterator i = value.begin(); i != value.end(); ++i)
		m_value.push_back((*i)->getName());
}

PropertyTypeSet::value_type_t PropertyTypeSet::get(const PropertyValue* value)
{
	value_type_t typeSet;
	if (value)
	{
		const std::vector< std::wstring >& tmp = checked_type_cast< const PropertyTypeSet* >(value)->m_value;
		for (std::vector< std::wstring >::const_iterator i = tmp.begin(); i != tmp.end(); ++i)
		{
			if (const Type* type = Type::find(*i))
				typeSet.insert(type);
		}
	}
	return typeSet;
}

bool PropertyTypeSet::serialize(Serializer& s)
{
	return s >> MemberStlVector< std::wstring >(L"value", m_value);
}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.editor.PropertyVector4", PropertyVector4, PropertyValue)

PropertyVector4::PropertyVector4(const value_type_t& value)
:	m_value(value)
{
}

PropertyVector4::value_type_t PropertyVector4::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertyVector4* >(value)->m_value : value_type_t();
}

bool PropertyVector4::serialize(Serializer& s)
{
	return s >> Member< value_type_t >(L"value", m_value);
}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.editor.PropertyQuaternion", PropertyQuaternion, PropertyValue)

PropertyQuaternion::PropertyQuaternion(const value_type_t& value)
:	m_value(value)
{
}

PropertyQuaternion::value_type_t PropertyQuaternion::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertyQuaternion* >(value)->m_value : value_type_t();
}

bool PropertyQuaternion::serialize(Serializer& s)
{
	return s >> Member< value_type_t >(L"value", m_value);
}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.editor.PropertyKey", PropertyKey, PropertyValue)

PropertyKey::PropertyKey(const value_type_t& value)
:	m_value(value)
{
}

PropertyKey::value_type_t PropertyKey::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertyKey* >(value)->m_value : value_type_t(0, 0);
}

bool PropertyKey::serialize(Serializer& s)
{
	if (s.getDirection() == Serializer::SdRead)
	{
		std::wstring keydesc = L"";

		s >> Member< std::wstring >(L"key", keydesc);

		if (keydesc.empty())
			return false;

		size_t pos = keydesc.find_first_of(L',');

		std::wstring state = pos != keydesc.npos ? keydesc.substr(0, pos) : L"";
		std::wstring key = pos != keydesc.npos ? keydesc.substr(pos + 1) : keydesc;

		if (key.empty())
			return false;

		m_value.first = 0;

		std::vector< std::wstring > states;
		Split< std::wstring >::any(state, L"|", states);
		for (std::vector< std::wstring >::iterator i = states.begin(); i != states.end(); ++i)
		{
			if (*i == L"KsControl")
				m_value.first |= ui::KsControl;
			else if (*i == L"KsMenu")
				m_value.first |= ui::KsMenu;
			else if (*i == L"KsShift")
				m_value.first |= ui::KsShift;
			else
				return false;
		}

		m_value.second = ui::Application::getInstance().translateVirtualKey(key);
	}
	else	// SdWrite
	{
		std::wstring keydesc = L"";

		if (m_value.first)
		{
			if (m_value.first & ui::KsControl)
				keydesc += L"KsControl";
			if (m_value.first & ui::KsMenu)
			{
				if (!keydesc.empty())
					keydesc += L"|";
				keydesc += L"KsMenu";
			}
			if (m_value.first & ui::KsShift)
			{
				if (!keydesc.empty())
					keydesc += L"|";
				keydesc += L"KsShift";
			}
		}

		if (!keydesc.empty())
			keydesc += L",";
		keydesc += ui::Application::getInstance().translateVirtualKey(m_value.second);

		s >> Member< std::wstring >(L"key", keydesc);
	}
	return true;
}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.editor.PropertyGroup", PropertyGroup, PropertyValue)

PropertyGroup::PropertyGroup(const value_type_t& value)
:	m_value(value)
{
}

PropertyGroup::value_type_t PropertyGroup::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertyGroup* >(value)->m_value : value_type_t();
}

void PropertyGroup::setProperty(const std::wstring& propertyName, PropertyValue* value)
{
	if (value)
		m_value[propertyName] = value;
	else
		m_value.erase(propertyName);
}

const PropertyValue* PropertyGroup::getProperty(const std::wstring& propertyName) const
{
	std::map< std::wstring, Ref< PropertyValue > >::const_iterator it = m_value.find(propertyName);
	return it != m_value.end() ? it->second.getPtr() : 0;
}

bool PropertyGroup::serialize(Serializer& s)
{
	return s >> MemberStlMap< std::wstring, Ref< PropertyValue >, MemberStlPair< std::wstring, Ref< PropertyValue >, Member< std::wstring >, MemberRef< PropertyValue > > >(L"value", m_value);
}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.editor.PropertySerializable", PropertySerializable, PropertyValue)

PropertySerializable::PropertySerializable(const value_type_t& value)
:	m_value(value)
{
}

PropertySerializable::value_type_t PropertySerializable::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertySerializable* >(value)->m_value.getPtr() : 0;
}

bool PropertySerializable::serialize(Serializer& s)
{
	return s >> MemberRef< Serializable >(L"value", m_value);
}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.Settings", Settings, Object)

Settings::Settings(PropertyGroup* globalGroup, PropertyGroup* userGroup)
:	m_globalGroup(globalGroup)
,	m_userGroup(userGroup)
{
}

void Settings::setProperty(const std::wstring& propertyName, PropertyValue* value)
{
	const PropertyValue* globalValue = m_globalGroup->getProperty(propertyName);
	if (globalValue && DeepHash(globalValue) == DeepHash(value))
	{
		// Value identical with global value, erase value from user configuration.
		m_userGroup->setProperty(propertyName, 0);
		return;
	}

	// Value not identical with global value, override value through user configuration.
	m_userGroup->setProperty(propertyName, value);
}

const PropertyValue* Settings::getProperty(const std::wstring& propertyName) const
{
	const PropertyValue* value = m_userGroup->getProperty(propertyName);
	if (!value)
		value = m_globalGroup->getProperty(propertyName);
	return value;
}

	}
}
