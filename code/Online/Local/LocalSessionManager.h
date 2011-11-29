#ifndef traktor_online_LocalSessionManager_H
#define traktor_online_LocalSessionManager_H

#include "Online/Local/LocalTypes.h"
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
class LocalSaveData;
class LocalStatistics;

class T_DLLCLASS LocalSessionManager : public ISessionManagerProvider
{
	T_RTTI_CLASS;

public:
	bool create(const LocalCreateDesc& desc);

	virtual void destroy();

	virtual bool update();

	virtual std::wstring getLanguageCode() const;

	virtual bool isConnected() const;

	virtual bool requireUserAttention() const;

	virtual bool haveP2PData() const;

	virtual uint32_t receiveP2PData(void* data, uint32_t size, uint64_t& outFromUserHandle) const;

	virtual IAchievementsProvider* getAchievements() const;

	virtual ILeaderboardsProvider* getLeaderboards() const;

	virtual IMatchMakingProvider* getMatchMaking() const;

	virtual ISaveDataProvider* getSaveData() const;

	virtual IStatisticsProvider* getStatistics() const;

	virtual IUserProvider* getUser() const;

private:
	Ref< sql::IConnection > m_db;
	Ref< LocalAchievements > m_achievements;
	Ref< LocalLeaderboards > m_leaderboards;
	Ref< LocalSaveData > m_saveData;
	Ref< LocalStatistics > m_statistics;
};

	}
}

#endif	// traktor_online_LocalSessionManager_H
