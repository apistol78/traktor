/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>

namespace traktor::online
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
