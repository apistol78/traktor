#pragma once

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

	virtual void execute(TaskQueue* taskQueue) override final;

private:
	Ref< IMatchMakingProvider > m_matchMakingProvider;
	Ref< UserCache > m_userCache;
	Ref< const LobbyFilter > m_filter;
	Ref< LobbyArrayResult > m_result;
};

	}
}

