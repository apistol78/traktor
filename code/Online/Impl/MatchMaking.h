/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Online/IMatchMaking.h"

namespace traktor
{
	namespace online
	{

class IMatchMakingProvider;
class TaskQueue;
class UserCache;

class MatchMaking : public IMatchMaking
{
	T_RTTI_CLASS;

public:
	virtual bool ready() const override final;

	virtual Ref< LobbyArrayResult > findMatchingLobbies(const LobbyFilter* filter) override final;

	virtual Ref< LobbyResult > createLobby(uint32_t maxUsers, LobbyAccess access) override final;

	virtual Ref< ILobby > acceptLobby() override final;

	virtual Ref< PartyResult > createParty() override final;

	virtual Ref< IParty > acceptParty() override final;

private:
	friend class SessionManager;

	Ref< IMatchMakingProvider > m_matchMakingProvider;
	Ref< UserCache > m_userCache;
	Ref< TaskQueue > m_taskQueue;

	MatchMaking(IMatchMakingProvider* matchMakingProvider, UserCache* userCache, TaskQueue* taskQueue);
};

	}
}


