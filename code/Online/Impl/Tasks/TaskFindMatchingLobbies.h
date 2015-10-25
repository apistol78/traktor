#ifndef traktor_online_TaskFindMatchingLobbies_H
#define traktor_online_TaskFindMatchingLobbies_H

#include "Online/Impl/ITask.h"

namespace traktor
{
	namespace online
	{

class IMatchMakingProvider;
class LobbyArrayResult;
class LobbyFilter;
class UserCache;

class TaskFindMatchingLobbies : public ITask
{
	T_RTTI_CLASS;

public:
	TaskFindMatchingLobbies(
		IMatchMakingProvider* matchMakingProvider,
		UserCache* userCache,
		const LobbyFilter* filter,
		LobbyArrayResult* result
	);

	virtual void execute(TaskQueue* taskQueue) T_OVERRIDE T_FINAL;

private:
	Ref< IMatchMakingProvider > m_matchMakingProvider;
	Ref< UserCache > m_userCache;
	Ref< const LobbyFilter > m_filter;
	Ref< LobbyArrayResult > m_result;
};

	}
}

#endif	// traktor_online_TaskFindMatchingLobbies_H
