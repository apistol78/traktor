#pragma once

#include <string>

namespace traktor
{
	namespace online
	{

enum LobbyAccess
{
	LaPublic = 0,
	LaPrivate = 1,
	LaFriends = 2
};

struct SaveDataDesc
{
	std::wstring title;
	std::wstring description;
};

	}
}

