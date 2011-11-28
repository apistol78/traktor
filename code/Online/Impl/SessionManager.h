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

class ISessionManagerProvider;
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

	virtual Ref< IAchievements > getAchievements() const;

	virtual Ref< ILeaderboards > getLeaderboards() const;

	virtual Ref< IMatchMaking > getMatchMaking() const;

	virtual Ref< ISaveData > getSaveData() const;

	virtual Ref< IStatistics > getStatistics() const;

private:
	Ref< ISessionManagerProvider > m_provider;
	Ref< TaskQueue > m_taskQueues[2];
	Ref< IAchievements > m_achievements;
	Ref< ILeaderboards > m_leaderboards;
	Ref< IMatchMaking > m_matchMaking;
	Ref< ISaveData > m_saveData;
	Ref< IStatistics > m_statistics;
};

	}
}

#endif	// traktor_online_SessionManager_H
