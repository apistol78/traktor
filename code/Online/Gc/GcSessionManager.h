#ifndef traktor_online_GcSessionManager_H
#define traktor_online_GcSessionManager_H

#include "Core/Ref.h"
#include "Online/Provider/ISessionManagerProvider.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_GC_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

class GcAchievements;
class GcLeaderboards;
class GcMatchMaking;
class GcSaveData;
class GcStatistics;
class GcUser;

class T_DLLCLASS GcSessionManager : public ISessionManagerProvider
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

private:
	Ref< GcAchievements > m_achievements;
	Ref< GcLeaderboards > m_leaderboards;
	Ref< GcMatchMaking > m_matchMaking;
	Ref< GcSaveData > m_saveData;
	Ref< GcStatistics > m_statistics;
	Ref< GcUser > m_user;
};

	}
}

#endif	// traktor_online_GcSessionManager_H
