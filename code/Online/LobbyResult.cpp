/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Online/LobbyResult.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LobbyResult", LobbyResult, Result)

void LobbyResult::succeed(ILobby* lobby)
{
	m_lobby = lobby;
	Result::succeed();
}

ILobby* LobbyResult::get() const
{
	wait();
	return m_lobby;
}

	}
}
