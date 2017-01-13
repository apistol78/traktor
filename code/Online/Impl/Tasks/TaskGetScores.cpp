#include "Online/Score.h"
#include "Online/ScoreArrayResult.h"
#include "Online/Impl/User.h"
#include "Online/Impl/UserCache.h"
#include "Online/Impl/Tasks/TaskGetScores.h"
#include "Online/Provider/ILeaderboardsProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskGetScores", TaskGetScores, ITask)

TaskGetScores::TaskGetScores(
	ILeaderboardsProvider* leaderboardProvider,
	UserCache* userCache,
	uint64_t handle,
	int32_t from,
	int32_t to,
	bool friends,
	ScoreArrayResult* result
)
:	m_leaderboardProvider(leaderboardProvider)
,	m_userCache(userCache)
,	m_handle(handle)
,	m_from(from)
,	m_to(to)
,	m_friends(friends)
,	m_result(result)
{
}

void TaskGetScores::execute(TaskQueue* taskQueue)
{
	std::vector< ILeaderboardsProvider::ScoreData > providerScores;
	RefArray< Score > scores;

	bool result = false;
	if (!m_friends)
		result = m_leaderboardProvider->getGlobalScores(m_handle, m_from, m_to, providerScores);
	else
		result = m_leaderboardProvider->getFriendScores(m_handle, m_from, m_to, providerScores);

	if (result)
	{
		scores.reserve(providerScores.size());
		for (std::vector< ILeaderboardsProvider::ScoreData >::const_iterator i = providerScores.begin(); i != providerScores.end(); ++i)
		{
			Ref< IUser > user = m_userCache->get(i->handle);
			if (user)
				scores.push_back(new Score(user, i->score, i->rank));
		}
		m_result->succeed(scores);
	}
	else
		m_result->fail();
}

	}
}
