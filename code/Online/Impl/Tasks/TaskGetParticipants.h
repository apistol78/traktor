#ifndef traktor_online_TaskGetParticipants_H
#define traktor_online_TaskGetParticipants_H

#include "Online/Impl/ITask.h"

namespace traktor
{
	namespace online
	{

class IMatchMakingProvider;
class IUserProvider;
class UserArrayResult;

class TaskGetParticipants : public ITask
{
	T_RTTI_CLASS;

public:
	TaskGetParticipants(
		IMatchMakingProvider* matchMakingProvider,
		IUserProvider* userProvider,
		uint64_t lobbyHandle,
		UserArrayResult* result
	);

	virtual void execute(TaskQueue* taskQueue);

private:
	Ref< IMatchMakingProvider > m_matchMakingProvider;
	Ref< IUserProvider > m_userProvider;
	uint64_t m_lobbyHandle;
	Ref< UserArrayResult > m_result;
};

	}
}

#endif	// traktor_online_TaskGetParticipants_H
