#include "Editor/Settings.h"
#include "Ui/Application.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberType.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Misc/String.h"
#include "Core/Misc/Split.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PropertyValue", PropertyValue, ISerializable)

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.PropertyBoolean", 0, PropertyBoolean, PropertyValue)

PropertyBoolean::PropertyBoolean(value_type_t value)
:	m_value(value)
{
}

PropertyBoolean::value_type_t PropertyBoolean::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertyBoolean* >(value)->m_value : false;
}

bool PropertyBoolean::serialize(ISerializer& s)
{
	return s >> Member< value_type_t >(L"value", m_value);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.PropertyInteger", 0, PropertyInteger, PropertyValue)

PropertyInteger::PropertyInteger(value_type_t value)
:	m_value(value)
{
}

PropertyInteger::value_type_t PropertyInteger::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertyInteger* >(value)->m_value : false;
}

bool PropertyInteger::serialize(ISerializer& s)
{
	return s >> Member< value_type_t >(L"value", m_value);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.PropertyFloat", 0, PropertyFloat, PropertyValue)

PropertyFloat::PropertyFloat(value_type_t value)
:	m_value(value)
{
}

PropertyFloat::value_type_t PropertyFloat::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertyFloat* >(value)->m_value : false;
}

bool PropertyFloat::serialize(ISerializer& s)
{
	return s >> Member< value_type_t >(L"value", m_value);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.PropertyString", 0, PropertyString, PropertyValue)

PropertyString::PropertyString(value_type_t value)
:	m_value(value)
{
}

PropertyString::value_type_t PropertyString::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertyString* >(value)->m_value : L"";
}

bool PropertyString::serialize(ISerializer& s)
{
	return s >> Member< value_type_t >(L"value", m_value);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.PropertyStringArray", 0, PropertyStringArray, PropertyValue)

PropertyStringArray::PropertyStringArray(const value_type_t& value)
:	m_value(value)
{
}

PropertyStringArray::value_type_t PropertyStringArray::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertyStringArray* >(value)->m_value : value_type_t();
}

bool PropertyStringArray::serialize(ISerializer& s)
{
	return s >> MemberStlVector< std::wstring >(L"value", m_value);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.PropertyGuid", 0, PropertyGuid, PropertyValue)

PropertyGuid::PropertyGuid(const value_type_t& value)
:	m_value(value)
{
}

PropertyGuid::value_type_t PropertyGuid::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertyGuid* >(value)->m_value : value_type_t();
}

bool PropertyGuid::serialize(ISerializer& s)
{
	return s >> Member< Guid >(L"value", m_value);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.PropertyGuidArray", 0, PropertyGuidArray, PropertyValue)

PropertyGuidArray::PropertyGuidArray(const value_type_t& value)
:	m_value(value)
{
}

PropertyGuidArray::value_type_t PropertyGuidArray::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertyGuidArray* >(value)->m_value : value_type_t();
}

bool PropertyGuidArray::serialize(ISerializer& s)
{
	return s >> MemberStlVector< Guid >(L"value", m_value);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.PropertyType", 0, PropertyType, PropertyValue)

PropertyType::PropertyType(value_type_t value)
:	m_value(value)
{
}

PropertyType::value_type_t PropertyType::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertyType* >(value)->m_value : 0;
}

bool PropertyType::serialize(ISerializer& s)
{
	return s >> MemberType(L"value", m_value);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.PropertyTypeSet", 0, PropertyTypeSet, PropertyValue)

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
			if (const TypeInfo* type = TypeInfo::find(*i))
				typeSet.insert(type);
		}
	}
	return typeSet;
}

bool PropertyTypeSet::serialize(ISerializer& s)
{
	return s >> MemberStlVector< std::wstring >(L"value", m_value);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.PropertyVector4", 0, PropertyVector4, PropertyValue)

PropertyVector4::PropertyVector4(const value_type_t& value)
:	m_value(value)
{
}

PropertyVector4::value_type_t PropertyVector4::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertyVector4* >(value)->m_value : value_type_t();
}

bool PropertyVector4::serialize(ISerializer& s)
{
	return s >> Member< value_type_t >(L"value", m_value);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.PropertyQuaternion", 0, PropertyQuaternion, PropertyValue)

PropertyQuaternion::PropertyQuaternion(const value_type_t& value)
:	m_value(value)
{
}

PropertyQuaternion::value_type_t PropertyQuaternion::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertyQuaternion* >(value)->m_value : value_type_t();
}

bool PropertyQuaternion::serialize(ISerializer& s)
{
	return s >> Member< value_type_t >(L"value", m_value);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.PropertyKey", 0, PropertyKey, PropertyValue)

PropertyKey::PropertyKey(const value_type_t& value)
:	m_value(value)
{
}

PropertyKey::value_type_t PropertyKey::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertyKey* >(value)->m_value : value_type_t(0, ui::VkNull);
}

bool PropertyKey::serialize(ISerializer& s)
{
	if (s.getDirection() == ISerializer::SdRead)
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

		m_value.second = ui::Application::getInstance()->translateVirtualKey(key);
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
		keydesc += ui::Application::getInstance()->translateVirtualKey(m_value.second);

		s >> Member< std::wstring >(L"key", keydesc);
	}
	return true;
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.PropertyGroup", 0, PropertyGroup, PropertyValue)

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

Ref< PropertyValue > PropertyGroup::getProperty(const std::wstring& propertyName)
{
	std::map< std::wstring, Ref< PropertyValue > >::iterator it = m_value.find(propertyName);
	return it != m_value.end() ? it->second.ptr() : 0;
}

Ref< const PropertyValue > PropertyGroup::getProperty(const std::wstring& propertyName) const
{
	std::map< std::wstring, Ref< PropertyValue > >::const_iterator it = m_value.find(propertyName);
	return it != m_value.end() ? it->second.ptr() : 0;
}

bool PropertyGroup::serialize(ISerializer& s)
{
	return s >> MemberStlMap<
		std::wstring,
		Ref< PropertyValue >,
		MemberStlPair<
			std::wstring,
			Ref< PropertyValue >,
			Member< std::wstring >,
			MemberRef< PropertyValue >
		>
	>(L"value", m_value);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.PropertySerializable", 0, PropertySerializable, PropertyValue)

PropertySerializable::PropertySerializable(const value_type_t& value)
:	m_value(value)
{
}

PropertySerializable::value_type_t PropertySerializable::get(const PropertyValue* value)
{
	return value ? checked_type_cast< const PropertySerializable* >(value)->m_value.ptr() : 0;
}

bool PropertySerializable::serialize(ISerializer& s)
{
	return s >> MemberRef< ISerializable >(L"value", m_value);
}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.Settings", Settings, Object)

Settings::Settings(PropertyGroup* globalGroup, PropertyGroup* userGroup)
:	m_globalGroup(globalGroup)
,	m_userGroup(userGroup)
{
}

void Settings::setProperty(const std::wstring& propertyName, PropertyValue* value)
{
	Ref< const PropertyValue > globalValue = m_globalGroup->getProperty(propertyName);
	if (globalValue && DeepHash(globalValue) == DeepHash(value))
	{
		// Value identical with global value, erase value from user configuration.
		m_userGroup->setProperty(propertyName, 0);
		return;
	}

	// Value not identical with global value, override value through user configuration.
	m_userGroup->setProperty(propertyName, value);
}

Ref< PropertyValue > Settings::getProperty(const std::wstring& propertyName)
{
	Ref< PropertyValue > value = m_userGroup->getProperty(propertyName);
	if (!value)
		value = m_globalGroup->getProperty(propertyName);
	return value;
}

Ref< const PropertyValue > Settings::getProperty(const std::wstring& propertyName) const
{
	Ref< const PropertyValue > value = m_userGroup->getProperty(propertyName);
	if (!value)
		value = m_globalGroup->getProperty(propertyName);
	return value;
}

	}
}
