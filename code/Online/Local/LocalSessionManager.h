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
#include "Online/Provider/ISessionManagerProvider.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_LOCAL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sql
	{

class IConnection;

	}

	namespace online
	{

class LocalAchievements;
class LocalLeaderboards;
class LocalMatchMaking;
class LocalSaveData;
class LocalStatistics;
class LocalUser;
class LocalVideoSharing;

class T_DLLCLASS LocalSessionManager : public ISessionManagerProvider
{
	T_RTTI_CLASS;

public:
	virtual bool create(const IGameConfiguration* configuration) override final;

	virtual void destroy() override final;

	virtual bool update() override final;

	virtual std::wstring getLanguageCode() const override final;

	virtual bool isConnected() const override final;

	virtual bool requireFullScreen() const override final;

	virtual bool requireUserAttention() const override final;

	virtual bool haveDLC(const std::wstring& id) const override final;

	virtual bool buyDLC(const std::wstring& id) const override final;

	virtual bool navigateUrl(const net::Url& url) const override final;

	virtual uint64_t getCurrentUserHandle() const override final;

	virtual bool getFriends(std::vector< uint64_t >& outFriends, bool onlineOnly) const override final;

	virtual bool findFriend(const std::wstring& name, uint64_t& outFriendUserHandle) const override final;

	virtual bool haveP2PData() const override final;

	virtual uint32_t receiveP2PData(void* data, uint32_t size, uint64_t& outFromUserHandle) const override final;

	virtual uint32_t getCurrentGameCount() const override final;

	virtual IAchievementsProvider* getAchievements() const override final;

	virtual ILeaderboardsProvider* getLeaderboards() const override final;

	virtual IMatchMakingProvider* getMatchMaking() const override final;

	virtual ISaveDataProvider* getSaveData() const override final;

	virtual IStatisticsProvider* getStatistics() const override final;

	virtual IUserProvider* getUser() const override final;

	virtual IVideoSharingProvider* getVideoSharing() const override final;

	virtual IVoiceChatProvider* getVoiceChat() const override final;

private:
	Ref< sql::IConnection > m_db;
	Ref< LocalAchievements > m_achievements;
	Ref< LocalLeaderboards > m_leaderboards;
	Ref< LocalMatchMaking > m_matchMaking;
	Ref< LocalSaveData > m_saveData;
	Ref< LocalStatistics > m_statistics;
	Ref< LocalUser > m_user;
	Ref< LocalVideoSharing > m_videoSharing;
};

	}
}

