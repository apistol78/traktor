#ifndef traktor_online_LeaderboardLocal_H
#define traktor_online_LeaderboardLocal_H

#include "Online/ILeaderboard.h"

namespace traktor
{
	namespace sql
	{

class IConnection;

	}

	namespace online
	{

class LeaderboardLocal : public ILeaderboard
{
	T_RTTI_CLASS;

public:
	LeaderboardLocal(sql::IConnection* db, int32_t leaderboardId, int32_t userId);

	virtual uint32_t getRank() const;

	virtual bool setScore(int32_t score);

	virtual int32_t getScore() const;

private:
	Ref< sql::IConnection > m_db;
	int32_t m_leaderboardId;
	int32_t m_userId;
};

	}
}

#endif	// traktor_online_LeaderboardLocal_H
