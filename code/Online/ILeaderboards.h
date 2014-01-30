#ifndef traktor_online_ILeaderboards_H
#define traktor_online_ILeaderboards_H

#include "Online/Result.h"
#include "Online/ScoreArrayResult.h"

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

class T_DLLCLASS ILeaderboards : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool ready() const = 0;

	virtual bool enumerate(std::set< std::wstring >& outLeaderboardIds) const = 0;

	virtual bool getRank(const std::wstring& leaderboardId, uint32_t& outRank) const = 0;

	virtual bool getScore(const std::wstring& leaderboardId, int32_t& outScore) const = 0;

	virtual Ref< Result > setScore(const std::wstring& leaderboardId, int32_t score) = 0;

	virtual Ref< Result > addScore(const std::wstring& leaderboardId, int32_t score) = 0;

	virtual Ref< ScoreArrayResult > getGlobalScores(const std::wstring& leaderboardId, int32_t from, int32_t to) = 0;

	virtual Ref< ScoreArrayResult > getFriendScores(const std::wstring& leaderboardId, int32_t from, int32_t to) = 0;
};

	}
}

#endif	// traktor_online_ILeaderboards_H
