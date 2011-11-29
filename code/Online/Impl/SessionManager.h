#ifndef traktor_online_SessionManager_H
#define traktor_online_SessionManager_H

#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
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

class ISessionManagerProvider;
class IUser;
class TaskQueue;

class T_DLLCLASS SessionManager : public ISessionManager
{
	T_RTTI_CLASS;

public:
	bool create(ISessionManagerProvider* provider);

	virtual void destroy();

	virtual bool update();

	virtual std::wstring getLanguageCode() const;

	virtual bool isConnected() const;

	virtual bool requireUserAttention() const;

	virtual bool haveP2PData() const;

	virtual uint32_t receiveP2PData(void* data, uint32_t size, Ref< IUser >& outFromUser) const;

	virtual IAchievements* getAchievements() const;

	virtual ILeaderboards* getLeaderboards() const;

	virtual IMatchMaking* getMatchMaking() const;

	virtual ISaveData* getSaveData() const;

	virtual IStatistics* getStatistics() const;

private:
	Ref< ISessionManagerProvider > m_provider;
	Ref< TaskQueue > m_taskQueues[2];
	Ref< IAchievements > m_achievements;
	Ref< ILeaderboards > m_leaderboards;
	Ref< IMatchMaking > m_matchMaking;
	Ref< ISaveData > m_saveData;
	Ref< IStatistics > m_statistics;
	mutable SmallMap< uint64_t, Ref< IUser > > m_p2pUsers;
};

	}
}

#endif	// traktor_online_SessionManager_H
