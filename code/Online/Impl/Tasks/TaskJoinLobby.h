#pragma once

#include "Online/Impl/ITask.h"

namespace traktor
{

class Result;

	namespace online
	{

class IMatchMakingProvider;

class TaskJoinLobby : public ITask
{
	T_RTTI_CLASS;

public:
	TaskJoinLobby(
		IMatchMakingProvider* provider,
		uint64_t lobbyHandle,
		Result* result
	);

	virtual void execute(TaskQueue* taskQueue) override final;

private:
	Ref< IMatchMakingProvider > m_provider;
	uint64_t m_lobbyHandle;
	Ref< Result > m_result;
};

	}
}

