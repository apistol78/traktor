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
#include <map>
#include <string>
#include "Online/IGameConfiguration.h"

namespace traktor
{
	namespace online
	{

class SteamGameConfiguration : public IGameConfiguration
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

private:
	friend class SteamSessionManager;

	uint32_t m_appId = 0;
	uint32_t m_requestAttempts = 10;
	bool m_drmEnabled = false;
	bool m_cloudEnabled = false;
	bool m_allowP2PRelay = true;
	std::list< std::wstring > m_achievementIds;
	std::list< std::wstring > m_leaderboardIds;
	std::list< std::wstring > m_statsIds;
	std::map< std::wstring, uint32_t > m_dlcIds;
};

	}
}

