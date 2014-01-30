#ifndef traktor_online_LocalLeaderboards_H
#define traktor_online_LocalLeaderboards_H

#include "Online/Provider/ILeaderboardsProvider.h"

namespace traktor
{
	namespace sql
	{

class IConnection;

	}

	namespace online
	{

class LocalLeaderboards : public ILeaderboardsProvider
{
	T_RTTI_CLASS;

public:
	LocalLeaderboards(sql::IConnection* db);

	virtual bool enumerate(std::map< std::wstring, LeaderboardData >& outLeaderboards);

	virtual bool set(uint64_t handle, int32_t score);

	virtual bool getGlobalScores(uint64_t handle, int32_t from, int32_t to, std::vector< std::pair< uint64_t, int32_t > >& outScores);

	virtual bool getFriendScores(uint64_t handle, int32_t from, int32_t to, std::vector< std::pair< uint64_t, int32_t > >& outScores);

private:
	Ref< sql::IConnection > m_db;
};

	}
}

#endif	// traktor_online_LocalLeaderboards_H
