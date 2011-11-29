#include "Online/UserArrayResult.h"
#include "Online/Impl/User.h"
#include "Online/Impl/Tasks/TaskGetParticipants.h"
#include "Online/Provider/IMatchMakingProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskGetParticipants", TaskGetParticipants, ITask)

TaskGetParticipants::TaskGetParticipants(
	IMatchMakingProvider* matchMakingProvider,
	IUserProvider* userProvider,
	uint64_t lobbyHandle,
	UserArrayResult* result
)
:	m_matchMakingProvider(matchMakingProvider)
,	m_userProvider(userProvider)
,	m_lobbyHandle(lobbyHandle)
,	m_result(result)
{
}

void TaskGetParticipants::execute(TaskQueue* taskQueue)
{
	T_ASSERT (m_matchMakingProvider);
	T_ASSERT (m_userProvider);
	T_ASSERT (m_result);

	std::vector< uint64_t > userHandles;
	if (m_matchMakingProvider->getParticipants(m_lobbyHandle, userHandles))
	{
		RefArray< IUser > users;
		users.reserve(userHandles.size());
		for (std::vector< uint64_t >::iterator i = userHandles.begin(); i != userHandles.end(); ++i)
			users.push_back(new User(m_userProvider, *i));
		m_result->succeed(users);
	}
	else
		m_result->fail();
}

	}
}
