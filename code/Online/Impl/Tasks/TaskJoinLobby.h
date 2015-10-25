#ifndef traktor_online_TaskJoinLobby_H
#define traktor_online_TaskJoinLobby_H

#include "Online/Impl/ITask.h"

namespace traktor
{
	namespace online
	{

class IMatchMakingProvider;
class Result;

class TaskJoinLobby : public ITask
{
	T_RTTI_CLASS;

public:
	TaskJoinLobby(
		IMatchMakingProvider* provider,
		uint64_t lobbyHandle,
		Result* result
	);

	virtual void execute(TaskQueue* taskQueue) T_OVERRIDE T_FINAL;

private:
	Ref< IMatchMakingProvider > m_provider;
	uint64_t m_lobbyHandle;
	Ref< Result > m_result;
};

	}
}

#endif	// traktor_online_TaskJoinLobby_H
