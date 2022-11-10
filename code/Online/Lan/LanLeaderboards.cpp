/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Online/Lan/LanLeaderboards.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LanLeaderboards", LanLeaderboards, ILeaderboardsProvider)

bool LanLeaderboards::enumerate(std::map< std::wstring, LeaderboardData >& outLeaderboards)
{
	return true;
}

bool LanLeaderboards::create(const std::wstring& leaderboardId, LeaderboardData& outLeaderboard)
{
	return false;
}

bool LanLeaderboards::set(const uint64_t handle, int32_t score)
{
	return true;
}

bool LanLeaderboards::getGlobalScores(uint64_t handle, int32_t from, int32_t to, std::vector< ScoreData >& outScores)
{
	return false;
}

bool LanLeaderboards::getFriendScores(uint64_t handle, int32_t from, int32_t to, std::vector< ScoreData >& outScores)
{
	return false;
}

	}
}
