#pragma once

#include "Online/Types.h"
#include "Online/Impl/ITask.h"

namespace traktor
{
	namespace online
	{

class IMatchMakingProvider;
class LobbyResult;
class UserCache;

class TaskCreateLobby : public ITask
{
	T_RTTI_CLASS;

public:
	TaskCreateLobby(
		IMatchMakingProvider* matchMakingProvider,
		UserCache* userCache,
		uint32_t maxUsers,
		LobbyAccess access,
		LobbyResult* result
	);

	virtual void execute(TaskQueue* taskQueue) override final;

private:
	Ref< IMatchMakingProvider > m_matchMakingProvider;
	Ref< UserCache > m_userCache;
	uint32_t m_maxUsers;
	LobbyAccess m_access;
	Ref< LobbyResult > m_result;
};

	}
}

