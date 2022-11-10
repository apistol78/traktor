/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
#include "Core/Object.h"
#include "Online/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace online
	{

class LobbyFilter;

class T_DLLCLASS IMatchMakingProvider : public Object
{
	T_RTTI_CLASS;

public:
	//! \group Lobby interface.
	//! \{

	virtual bool findMatchingLobbies(const LobbyFilter* filter, std::vector< uint64_t >& outLobbyHandles) = 0;

	virtual bool createLobby(uint32_t maxUsers, LobbyAccess access, uint64_t& outLobbyHandle) = 0;

	virtual bool acceptLobby(uint64_t& outLobbyHandle) = 0;

	virtual bool joinLobby(uint64_t lobbyHandle) = 0;

	virtual bool leaveLobby(uint64_t lobbyHandle) = 0;

	virtual bool setLobbyMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value) = 0;

	virtual bool getLobbyMetaValue(uint64_t lobbyHandle, const std::wstring& key, std::wstring& outValue) = 0;

	virtual bool setLobbyParticipantMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value) = 0;

	virtual bool getLobbyParticipantMetaValue(uint64_t lobbyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue) = 0;

	virtual bool getLobbyParticipants(uint64_t lobbyHandle, std::vector< uint64_t >& outUserHandles) = 0;

	virtual bool getLobbyParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const = 0;

	virtual bool getLobbyMaxParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const = 0;

	virtual bool getLobbyFriendsCount(uint64_t lobbyHandle, uint32_t& outCount) const = 0;

	virtual bool inviteToLobby(uint64_t lobbyHandle, uint64_t userHandle) = 0;

	virtual bool setLobbyOwner(uint64_t lobbyHandle, uint64_t userHandle) const = 0;

	virtual bool getLobbyOwner(uint64_t lobbyHandle, uint64_t& outUserHandle) const = 0;

	//\}

	//! \group Party interface.
	//! \{

	virtual bool createParty(uint64_t& outPartyHandle) = 0;

	virtual bool acceptParty(uint64_t& outPartyHandle) = 0;

	virtual bool joinParty(uint64_t partyHandle) = 0;

	virtual bool leaveParty(uint64_t partyHandle) = 0;

	virtual bool setPartyMetaValue(uint64_t partyHandle, const std::wstring& key, const std::wstring& value) = 0;

	virtual bool getPartyMetaValue(uint64_t partyHandle, const std::wstring& key, std::wstring& outValue) = 0;

	virtual bool setPartyParticipantMetaValue(uint64_t partyHandle, const std::wstring& key, const std::wstring& value) = 0;

	virtual bool getPartyParticipantMetaValue(uint64_t partyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue) = 0;

	virtual bool getPartyParticipants(uint64_t partyHandle, std::vector< uint64_t >& outUserHandles) = 0;

	virtual bool getPartyParticipantCount(uint64_t partyHandle, uint32_t& outCount) const = 0;

	virtual bool inviteToParty(uint64_t partyHandle, uint64_t userHandle) = 0;

	//! \}
};

	}
}

