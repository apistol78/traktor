/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_LanSessionManager_H
#define traktor_online_LanSessionManager_H

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

	virtual bool create(const IGameConfiguration* configuration) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool update() T_OVERRIDE T_FINAL;

	virtual std::wstring getLanguageCode() const T_OVERRIDE T_FINAL;

	virtual bool isConnected() const T_OVERRIDE T_FINAL;

	virtual bool requireFullScreen() const T_OVERRIDE T_FINAL;

	virtual bool requireUserAttention() const T_OVERRIDE T_FINAL;

	virtual bool haveDLC(const std::wstring& id) const T_OVERRIDE T_FINAL;

	virtual bool buyDLC(const std::wstring& id) const T_OVERRIDE T_FINAL;

	virtual bool navigateUrl(const net::Url& url) const T_OVERRIDE T_FINAL;

	virtual uint64_t getCurrentUserHandle() const T_OVERRIDE T_FINAL;

	virtual bool getFriends(std::vector< uint64_t >& outFriends, bool onlineOnly) const T_OVERRIDE T_FINAL;

	virtual bool findFriend(const std::wstring& name, uint64_t& outFriendUserHandle) const T_OVERRIDE T_FINAL;

	virtual bool haveP2PData() const T_OVERRIDE T_FINAL;

	virtual uint32_t receiveP2PData(void* data, uint32_t size, uint64_t& outFromUserHandle) const T_OVERRIDE T_FINAL;

	virtual uint32_t getCurrentGameCount() const T_OVERRIDE T_FINAL;

	virtual IAchievementsProvider* getAchievements() const T_OVERRIDE T_FINAL;

	virtual ILeaderboardsProvider* getLeaderboards() const T_OVERRIDE T_FINAL;

	virtual IMatchMakingProvider* getMatchMaking() const T_OVERRIDE T_FINAL;

	virtual ISaveDataProvider* getSaveData() const T_OVERRIDE T_FINAL;

	virtual IStatisticsProvider* getStatistics() const T_OVERRIDE T_FINAL;

	virtual IUserProvider* getUser() const T_OVERRIDE T_FINAL;

	virtual IVideoSharingProvider* getVideoSharing() const T_OVERRIDE T_FINAL;

	virtual IVoiceChatProvider* getVoiceChat() const T_OVERRIDE T_FINAL;

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

#endif	// traktor_online_LanSessionManager_H
