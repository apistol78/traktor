#ifndef traktor_online_SessionManager_H
#define traktor_online_SessionManager_H

#include "Core/RefArray.h"
#include "Online/ISessionManager.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

class IGameConfiguration;
class ISessionManagerProvider;
class IUser;
class TaskQueue;
class UserCache;

class T_DLLCLASS SessionManager : public ISessionManager
{
	T_RTTI_CLASS;

public:
	bool create(ISessionManagerProvider* provider, const IGameConfiguration* configuration);

	virtual void destroy();

	virtual bool update();

	virtual std::wstring getLanguageCode() const;

	virtual bool isConnected() const;

	virtual bool requireUserAttention() const;

	virtual bool haveDLC(const std::wstring& id) const;

	virtual bool getFriends(RefArray< IUser >& outFriends, bool onlineOnly) const;

	virtual bool findFriend(const std::wstring& name, Ref< IUser >& outFriend) const;

	virtual bool haveP2PData() const;

	virtual uint32_t receiveP2PData(void* data, uint32_t size, Ref< IUser >& outFromUser) const;

	virtual IAchievements* getAchievements() const;

	virtual ILeaderboards* getLeaderboards() const;

	virtual IMatchMaking* getMatchMaking() const;

	virtual ISaveData* getSaveData() const;

	virtual IStatistics* getStatistics() const;

	virtual IUser* getUser() const;

private:
	Ref< ISessionManagerProvider > m_provider;
	Ref< TaskQueue > m_taskQueues[2];
	Ref< IAchievements > m_achievements;
	Ref< ILeaderboards > m_leaderboards;
	Ref< IMatchMaking > m_matchMaking;
	Ref< ISaveData > m_saveData;
	Ref< IStatistics > m_statistics;
	Ref< IUser > m_user;
	Ref< UserCache > m_userCache;
};

	}
}

#endif	// traktor_online_SessionManager_H
