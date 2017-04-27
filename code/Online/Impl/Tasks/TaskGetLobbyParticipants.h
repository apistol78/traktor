/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_TaskGetLobbyParticipants_H
#define traktor_online_TaskGetLobbyParticipants_H

#include "Online/Impl/ITask.h"

namespace traktor
{
	namespace online
	{

class IMatchMakingProvider;
class UserArrayResult;
class UserCache;

class TaskGetLobbyParticipants : public ITask
{
	T_RTTI_CLASS;

public:
	TaskGetLobbyParticipants(
		IMatchMakingProvider* matchMakingProvider,
		UserCache* userCache,
		uint64_t lobbyHandle,
		UserArrayResult* result
	);

	virtual void execute(TaskQueue* taskQueue) T_OVERRIDE T_FINAL;

private:
	Ref< IMatchMakingProvider > m_matchMakingProvider;
	Ref< UserCache > m_userCache;
	uint64_t m_lobbyHandle;
	Ref< UserArrayResult > m_result;
};

	}
}

#endif	// traktor_online_TaskGetLobbyParticipants_H
