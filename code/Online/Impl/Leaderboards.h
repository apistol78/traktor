#ifndef traktor_online_Leaderboards_H
#define traktor_online_Leaderboards_H

#include <map>
#include "Core/Thread/Semaphore.h"
#include "Online/ILeaderboards.h"
#include "Online/Provider/ILeaderboardsProvider.h"

namespace traktor
{
	namespace online
	{

class ILeaderboardsProvider;
class TaskQueue;
class UserCache;

class Leaderboards : public ILeaderboards
{
	T_RTTI_CLASS;

public:
	virtual bool ready() const T_OVERRIDE T_FINAL;

	virtual bool enumerate(std::set< std::wstring >& outLeaderboardIds) const T_OVERRIDE T_FINAL;

	virtual bool getRank(const std::wstring& leaderboardId, uint32_t& outRank) const T_OVERRIDE T_FINAL;

	virtual bool getScore(const std::wstring& leaderboardId, int32_t& outScore) const T_OVERRIDE T_FINAL;

	virtual Ref< Result > setScore(const std::wstring& leaderboardId, int32_t score) T_OVERRIDE T_FINAL;

	virtual Ref< Result > addScore(const std::wstring& leaderboardId, int32_t score) T_OVERRIDE T_FINAL;

	virtual Ref< ScoreArrayResult > getGlobalScores(const std::wstring& leaderboardId, int32_t from, int32_t to) T_OVERRIDE T_FINAL;

	virtual Ref< ScoreArrayResult > getFriendScores(const std::wstring& leaderboardId, int32_t from, int32_t to) T_OVERRIDE T_FINAL;

private:
	friend class SessionManager;

	Ref< ILeaderboardsProvider > m_provider;
	Ref< UserCache > m_userCache;
	Ref< TaskQueue > m_taskQueue;
	mutable Semaphore m_lock;
	std::map< std::wstring, ILeaderboardsProvider::LeaderboardData > m_leaderboards;
	bool m_ready;

	Leaderboards(ILeaderboardsProvider* provider, UserCache* userCache, TaskQueue* taskQueue);

	void enqueueEnumeration();

	void callbackEnumLeaderboards(const std::map< std::wstring, ILeaderboardsProvider::LeaderboardData >& leaderboards);
};

	}
}

#endif	// traktor_online_Leaderboards_H
