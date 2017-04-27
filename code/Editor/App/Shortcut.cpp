/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/StringSplit.h"
#include "Editor/App/Shortcut.h"
#include "Ui/Application.h"

namespace traktor
{
	namespace editor
	{

std::pair< int, ui::VirtualKey > parseShortcut(const std::wstring& keyDesc)
{
	std::pair< int, ui::VirtualKey > value;
	value.first = 0;

	size_t pos = keyDesc.find_first_of(L',');
	std::wstring state = pos != keyDesc.npos ? keyDesc.substr(0, pos) : L"";
	std::wstring key = pos != keyDesc.npos ? keyDesc.substr(pos + 1) : keyDesc;

	if (!key.empty())
	{
		StringSplit< std::wstring > ss(state, L"|");
		for (StringSplit< std::wstring >::const_iterator i = ss.begin(); i != ss.end(); ++i)
		{
			if (*i == L"KsCommand")
				value.first |= ui::KsCommand;
			else if (*i == L"KsControl")
				value.first |= ui::KsControl;
			else if (*i == L"KsMenu")
				value.first |= ui::KsMenu;
			else if (*i == L"KsShift")
				value.first |= ui::KsShift;
			else
				return value;
		}

		value.second = ui::Application::getInstance()->translateVirtualKey(key);
	}

	return value;
}

std::wstring describeShortcut(const std::pair< int, ui::VirtualKey >& shortcut)
{
	std::wstring keyDesc;

	if (shortcut.first)
	{
		if (shortcut.first & ui::KsCommand)
			keyDesc = L"KsCommand";
		if (shortcut.first & ui::KsControl)
		{
			if (!keyDesc.empty())
				keyDesc += L"|";
			keyDesc += L"KsControl";
		}
		if (shortcut.first & ui::KsMenu)
		{
			if (!keyDesc.empty())
				keyDesc += L"|";
			keyDesc += L"KsMenu";
		}
		if (shortcut.first & ui::KsShift)
		{
			if (!keyDesc.empty())
				keyDesc += L"|";
			keyDesc += L"KsShift";
		}
	}

	if (!keyDesc.empty())
		keyDesc += L",";

	keyDesc += ui::Application::getInstance()->translateVirtualKey(shortcut.second);
	return keyDesc;
}

	}
}
