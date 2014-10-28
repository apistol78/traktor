#ifndef traktor_online_LocalSessionManager_H
#define traktor_online_LocalSessionManager_H

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

class T_DLLCLASS LocalSessionManager : public ISessionManagerProvider
{
	T_RTTI_CLASS;

public:
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

private:
	Ref< sql::IConnection > m_db;
	Ref< LocalAchievements > m_achievements;
	Ref< LocalLeaderboards > m_leaderboards;
	Ref< LocalMatchMaking > m_matchMaking;
	Ref< LocalSaveData > m_saveData;
	Ref< LocalStatistics > m_statistics;
	Ref< LocalUser > m_user;
};

	}
}

#endif	// traktor_online_LocalSessionManager_H
