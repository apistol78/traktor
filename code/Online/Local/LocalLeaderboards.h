/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Online/Provider/ILeaderboardsProvider.h"

namespace traktor
{
	namespace sql
	{

class IConnection;

	}

	namespace online
	{

class LocalLeaderboards : public ILeaderboardsProvider
{
	T_RTTI_CLASS;

public:
	LocalLeaderboards(sql::IConnection* db);

	virtual bool enumerate(std::map< std::wstring, LeaderboardData >& outLeaderboards) override final;

	virtual bool create(const std::wstring& leaderboardId, LeaderboardData& outLeaderboard) override final;

	virtual bool set(uint64_t handle, int32_t score) override final;

	virtual bool getGlobalScores(uint64_t handle, int32_t from, int32_t to, std::vector< ScoreData >& outScores) override final;

	virtual bool getFriendScores(uint64_t handle, int32_t from, int32_t to, std::vector< ScoreData >& outScores) override final;

private:
	Ref< sql::IConnection > m_db;
};

	}
}

