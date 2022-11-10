/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include <steam/steam_api.h>
#include "Core/Ref.h"
#include "Core/Thread/Semaphore.h"
#include "Online/Provider/ISessionManagerProvider.h"
#include "Online/Steam/SteamTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_STEAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace online
	{

class SteamAchievements;
class SteamLeaderboards;
class SteamMatchMaking;
class SteamSaveData;
class SteamStatistics;
class SteamUser;
class SteamVoiceChat;

class T_DLLCLASS SteamSessionManager : public ISessionManagerProvider
{
	T_RTTI_CLASS;

public:
	SteamSessionManager();

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

	bool waitForStats();

	bool storeStats();

private:
	Ref< SteamAchievements > m_achievements;
	Ref< SteamLeaderboards > m_leaderboards;
	Ref< SteamMatchMaking > m_matchMaking;
	Ref< ISaveDataProvider > m_saveData;
	Ref< SteamStatistics > m_statistics;
	Ref< SteamUser > m_user;
	Ref< SteamVoiceChat > m_voiceChat;
	std::map< std::wstring, uint32_t > m_dlcIds;
	bool m_requireUserAttention;
	bool m_requestedStats;
	bool m_receivedStats;
	bool m_receivedStatsSucceeded;
	uint32_t m_maxRequestAttempts;
	uint32_t m_requestAttempts;
	uint32_t m_updateGameCountTicks;
	uint32_t m_currentGameCount;
	CCallResult< SteamSessionManager, NumberOfCurrentPlayers_t > m_callbackGameCount;

	STEAM_CALLBACK(SteamSessionManager, OnUserStatsReceived, UserStatsReceived_t, m_callbackUserStatsReceived);

	STEAM_CALLBACK(SteamSessionManager, OnOverlayActivated, GameOverlayActivated_t, m_callbackOverlay);

	STEAM_CALLBACK(SteamSessionManager, OnP2PSessionRequest, P2PSessionRequest_t, m_callbackSessionRequest);

	void OnCurrentGameCount(NumberOfCurrentPlayers_t* pParam, bool bIOFailure);
};

	}
}

