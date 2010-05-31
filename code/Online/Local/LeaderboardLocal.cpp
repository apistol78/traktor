#include "Online/Local/LeaderboardLocal.h"
#include "Sql/IConnection.h"
#include "Sql/IResultSet.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LeaderboardLocal", LeaderboardLocal, ILeaderboard)

LeaderboardLocal::LeaderboardLocal(sql::IConnection* db, int32_t leaderboardId, int32_t userId)
:	m_db(db)
,	m_leaderboardId(leaderboardId)
,	m_userId(userId)
{
}

uint32_t LeaderboardLocal::getRank() const
{
	return 0;
}

bool LeaderboardLocal::setScore(int32_t score)
{
	if (m_db->executeUpdate(L"update LeaderboardEntries set score=" + toString(score) + L" where leaderboard=" + toString(m_leaderboardId) + L" and user=" + toString(m_userId)) > 0)
		return true;
	else if (m_db->executeUpdate(L"insert into LeaderboardEntries (leaderboard, user, score) values (" + toString(m_leaderboardId) + L", " + toString(m_userId) + L", " + toString(score) + L")") > 0)
		return true;
	else
		return false;
}

int32_t LeaderboardLocal::getScore() const
{
	Ref< sql::IResultSet > rs;

	rs = m_db->executeQuery(L"select score from LeaderboardEntries where leaderboard=" + toString(m_leaderboardId) + L" and user=" + toString(m_userId));
	if (!rs || !rs->next())
		return 0;

	return rs->getInt32(0);
}

	}
}
