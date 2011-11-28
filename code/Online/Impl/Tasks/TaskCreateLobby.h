#ifndef traktor_online_TaskCreateLobby_H
#define traktor_online_TaskCreateLobby_H

#include "Online/Impl/ITask.h"

namespace traktor
{
	namespace online
	{

class IMatchMakingProvider;
class LobbyResult;

class TaskCreateLobby : public ITask
{
	T_RTTI_CLASS;

public:
	TaskCreateLobby(
		IMatchMakingProvider* provider,
		uint32_t maxUsers,
		LobbyResult* result
	);

	virtual void execute(TaskQueue* taskQueue);

private:
	Ref< IMatchMakingProvider > m_provider;
	uint32_t m_maxUsers;
	Ref< LobbyResult > m_result;
};

	}
}

#endif	// traktor_online_TaskCreateLobby_H
