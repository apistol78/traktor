/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Online/Impl/ITask.h"

namespace traktor::online
{

class IMatchMakingProvider;
class UserArrayResult;
class UserCache;

class TaskGetLobbyParticipants : public ITask
{
	T_RTTI_CLASS;

public:
	explicit TaskGetLobbyParticipants(
		IMatchMakingProvider* matchMakingProvider,
		UserCache* userCache,
		uint64_t lobbyHandle,
		UserArrayResult* result
	);

	virtual void execute(TaskQueue* taskQueue) override final;

private:
	Ref< IMatchMakingProvider > m_matchMakingProvider;
	Ref< UserCache > m_userCache;
	uint64_t m_lobbyHandle;
	Ref< UserArrayResult > m_result;
};

}
