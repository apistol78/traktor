#ifndef traktor_online_TaskGetParticipants_H
#define traktor_online_TaskGetParticipants_H

#include "Online/Impl/ITask.h"

namespace traktor
{
	namespace online
	{

class IMatchMakingProvider;
class UserArrayResult;
class UserCache;

class TaskGetParticipants : public ITask
{
	T_RTTI_CLASS;

public:
	TaskGetParticipants(
		IMatchMakingProvider* matchMakingProvider,
		UserCache* userCache,
		uint64_t lobbyHandle,
		UserArrayResult* result
	);

	virtual void execute(TaskQueue* taskQueue);

private:
	Ref< IMatchMakingProvider > m_matchMakingProvider;
	Ref< UserCache > m_userCache;
	uint64_t m_lobbyHandle;
	Ref< UserArrayResult > m_result;
};

	}
}

#endif	// traktor_online_TaskGetParticipants_H
