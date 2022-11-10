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
#include <string>
#include "Online/IGameConfiguration.h"

namespace traktor
{
	namespace online
	{

class LocalGameConfiguration : public IGameConfiguration
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

private:
	friend class LocalSessionManager;

	std::wstring m_dbName;
	std::list< std::wstring > m_achievementIds;
	std::list< std::wstring > m_leaderboardIds;
	std::list< std::wstring > m_statsIds;
	std::list< std::wstring > m_dlcIds;
};

	}
}

