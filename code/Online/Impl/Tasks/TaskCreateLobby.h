/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Online/Types.h"
#include "Online/Impl/ITask.h"

namespace traktor::online
{

class IMatchMakingProvider;
class LobbyResult;
class UserCache;

class TaskCreateLobby : public ITask
{
	T_RTTI_CLASS;

public:
	explicit TaskCreateLobby(
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
