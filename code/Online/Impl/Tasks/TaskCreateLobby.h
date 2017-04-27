/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_TaskCreateLobby_H
#define traktor_online_TaskCreateLobby_H

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

	virtual void execute(TaskQueue* taskQueue) T_OVERRIDE T_FINAL;

private:
	Ref< IMatchMakingProvider > m_matchMakingProvider;
	Ref< UserCache > m_userCache;
	uint32_t m_maxUsers;
	LobbyAccess m_access;
	Ref< LobbyResult > m_result;
};

	}
}

#endif	// traktor_online_TaskCreateLobby_H
