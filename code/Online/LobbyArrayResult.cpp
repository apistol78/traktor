/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Online/LobbyArrayResult.h"

namespace traktor
{
	namespace online
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
}
