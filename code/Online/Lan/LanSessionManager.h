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

	}

	namespace online
	{

class LanAchievements;
class LanLeaderboards;
class LanMatchMaking;
class LanSaveData;
class LanStatistics;
class LanUser;

class T_DLLCLASS LanSessionManager : public ISessionManagerProvider
{
	T_RTTI_CLASS;

public:
	LanSessionManager();

	virtual bool create(const IGameConfiguration* configuration);

	virtual void destroy();

	virtual bool update();

	virtual std::wstring getLanguageCode() const;

	virtual bool isConnected() const;

	virtual bool requireUserAttention() const;

	virtual bool haveDLC(const std::wstring& id) const;

	virtual bool buyDLC(const std::wstring& id) const;

	virtual bool navigateUrl(const net::Url& url) const;

	virtual uint64_t getCurrentUserHandle() const;

	virtual bool getFriends(std::vector< uint64_t >& outFriends, bool onlineOnly) const;

	virtual bool findFriend(const std::wstring& name, uint64_t& outFriendUserHandle) const;

	virtual bool haveP2PData() const;

	virtual uint32_t receiveP2PData(void* data, uint32_t size, uint64_t& outFromUserHandle) const;

	virtual uint32_t getCurrentGameCount() const;

	virtual IAchievementsProvider* getAchievements() const;

	virtual ILeaderboardsProvider* getLeaderboards() const;

	virtual IMatchMakingProvider* getMatchMaking() const;

	virtual ISaveDataProvider* getSaveData() const;

	virtual IStatisticsProvider* getStatistics() const;

	virtual IUserProvider* getUser() const;

	virtual IVideoSharingProvider* getVideoSharing() const;

	virtual IVoiceChatProvider* getVoiceChat() const;

private:
	uint64_t m_userHandle;
	Ref< net::DiscoveryManager > m_discoveryManager;
	Ref< LanAchievements > m_achievements;
	Ref< LanLeaderboards > m_leaderboards;
	Ref< LanMatchMaking > m_matchMaking;
	Ref< LanSaveData > m_saveData;
	Ref< LanStatistics > m_statistics;
	Ref< LanUser > m_user;
};

	}
}

#endif	// traktor_online_LanSessionManager_H
