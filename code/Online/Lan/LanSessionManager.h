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
#if defined(T_ONLINE_LAN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class DiscoveryManager;
class UdpSocket;

	}

	namespace online
	{

class LanAchievements;
class LanLeaderboards;
class LanMatchMaking;
class LanSaveData;
class LanStatistics;
class LanUser;
class LanVoiceChat;

class T_DLLCLASS LanSessionManager : public ISessionManagerProvider
{
	T_RTTI_CLASS;

public:
	LanSessionManager();

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
	int32_t m_userHandle;
	Ref< net::DiscoveryManager > m_discoveryManager;
	Ref< net::UdpSocket > m_socket;
	uint16_t m_port;

	Ref< LanAchievements > m_achievements;
	Ref< LanLeaderboards > m_leaderboards;
	Ref< LanMatchMaking > m_matchMaking;
	Ref< LanSaveData > m_saveData;
	Ref< LanStatistics > m_statistics;
	Ref< LanUser > m_user;
	Ref< LanVoiceChat > m_voiceChat;
};

	}
}

