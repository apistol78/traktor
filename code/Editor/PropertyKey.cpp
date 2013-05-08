#include "Core/Misc/Split.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Editor/PropertyKey.h"
#include "Ui/Application.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.PropertyKey", 0, PropertyKey, IPropertyValue)

PropertyKey::PropertyKey(const value_type_t& value)
:	m_value(value)
{
}

PropertyKey::value_type_t PropertyKey::get(const IPropertyValue* value)
{
	return value ? checked_type_cast< const PropertyKey* >(value)->m_value : value_type_t(0, ui::VkNull);
}

void PropertyKey::serialize(ISerializer& s)
{
	if (s.getDirection() == ISerializer::SdRead)
	{
		std::wstring keydesc = L"";

		s >> Member< std::wstring >(L"key", keydesc);

		if (keydesc.empty())
			return;

		size_t pos = keydesc.find_first_of(L',');

		std::wstring state = pos != keydesc.npos ? keydesc.substr(0, pos) : L"";
		std::wstring key = pos != keydesc.npos ? keydesc.substr(pos + 1) : keydesc;

		if (key.empty())
			return;

		m_value.first = 0;

		std::vector< std::wstring > states;
		Split< std::wstring >::any(state, L"|", states);
		for (std::vector< std::wstring >::iterator i = states.begin(); i != states.end(); ++i)
		{
			if (*i == L"KsCommand")
				m_value.first |= ui::KsCommand;
			else if (*i == L"KsControl")
				m_value.first |= ui::KsControl;
			else if (*i == L"KsMenu")
				m_value.first |= ui::KsMenu;
			else if (*i == L"KsShift")
				m_value.first |= ui::KsShift;
			else
				return;
		}

		m_value.second = ui::Application::getInstance()->translateVirtualKey(key);
	}
	else	// SdWrite
	{
		std::wstring keydesc = L"";

		if (m_value.first)
		{
			if (m_value.first & ui::KsCommand)
				keydesc = L"KsCommand";
			if (m_value.first & ui::KsControl)
			{
				if (!keydesc.empty())
					keydesc += L"|";
				keydesc += L"KsControl";
			}
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
}

Ref< IPropertyValue > PropertyKey::join(const IPropertyValue* right) const
{
	return right->clone();
}

Ref< IPropertyValue > PropertyKey::clone() const
{
	return new PropertyKey(m_value);
}

	}
}
