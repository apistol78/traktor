/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_LanLeaderboards_H
#define traktor_online_LanLeaderboards_H

#include "Online/Provider/ILeaderboardsProvider.h"

namespace traktor
{
	namespace online
	{

class LanLeaderboards : public ILeaderboardsProvider
{
	T_RTTI_CLASS;

public:
	virtual bool enumerate(std::map< std::wstring, LeaderboardData >& outLeaderboards) override final;

	virtual bool create(const std::wstring& leaderboardId, LeaderboardData& outLeaderboard) override final;

	virtual bool set(uint64_t handle, int32_t score) override final;

	virtual bool getGlobalScores(uint64_t handle, int32_t from, int32_t to, std::vector< ScoreData >& outScores) override final;

	virtual bool getFriendScores(uint64_t handle, int32_t from, int32_t to, std::vector< ScoreData >& outScores) override final;
};

	}
}

#endif	// traktor_online_LanLeaderboards_H
