/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include <set>
#include <steam/steam_api.h>
#include "Online/Provider/IStatisticsProvider.h"
#include "Online/Steam/SteamTypes.h"

namespace traktor
{
	namespace online
	{

class SteamSessionManager;

class SteamStatistics : public IStatisticsProvider
{
	T_RTTI_CLASS;

public:
	SteamStatistics(SteamSessionManager* sessionManager, const std::list< std::wstring >& statIds);

	virtual bool enumerate(std::map< std::wstring, int32_t >& outStats) override final;

	virtual bool set(const std::wstring& statId, int32_t value) override final;

private:
	SteamSessionManager* m_sessionManager;
	std::set< std::wstring > m_statIds;

	STEAM_CALLBACK(SteamStatistics, OnUserStatsStored, UserStatsStored_t, m_callbackUserStatsStored);
};

	}
}

