/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Online/LobbyArrayResult.h"

namespace traktor::online
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LobbyArrayResult", LobbyArrayResult, Result)

void LobbyArrayResult::succeed(const RefArray< ILobby >& lobbies)
{
	m_lobbies = lobbies;
	Result::succeed();
}

const RefArray< ILobby >& LobbyArrayResult::get() const
{
	wait();
	return m_lobbies;
}

}
