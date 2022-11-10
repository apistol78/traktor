/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include "Online/Provider/IMatchMakingProvider.h"

namespace traktor
{
	namespace online
	{

class LocalMatchMaking : public IMatchMakingProvider
{
	T_RTTI_CLASS;

public:
	// Lobby

	virtual bool findMatchingLobbies(const LobbyFilter* filter, std::vector< uint64_t >& outLobbyHandles) override final;

	virtual bool createLobby(uint32_t maxUsers, LobbyAccess access, uint64_t& outLobbyHandle) override final;

	virtual bool acceptLobby(uint64_t& outLobbyHandle) override final;

	virtual bool joinLobby(uint64_t lobbyHandle) override final;

	virtual bool leaveLobby(uint64_t lobbyHandle) override final;

	virtual bool setLobbyMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value) override final;

	virtual bool getLobbyMetaValue(uint64_t lobbyHandle, const std::wstring& key, std::wstring& outValue) override final;

	virtual bool setLobbyParticipantMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value) override final;

	virtual bool getLobbyParticipantMetaValue(uint64_t lobbyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue) override final;

	virtual bool getLobbyParticipants(uint64_t lobbyHandle, std::vector< uint64_t >& outUserHandles) override final;

	virtual bool getLobbyParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const override final;

	virtual bool getLobbyMaxParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const override final;

	virtual bool getLobbyFriendsCount(uint64_t lobbyHandle, uint32_t& outCount) const override final;

	virtual bool inviteToLobby(uint64_t lobbyHandle, uint64_t userHandle) override final;

	virtual bool setLobbyOwner(uint64_t lobbyHandle, uint64_t userHandle) const override final;

	virtual bool getLobbyOwner(uint64_t lobbyHandle, uint64_t& outUserHandle) const override final;

	// Party

	virtual bool createParty(uint64_t& outPartyHandle) override final;

	virtual bool acceptParty(uint64_t& outPartyHandle) override final;

	virtual bool joinParty(uint64_t partyHandle) override final;

	virtual bool leaveParty(uint64_t partyHandle) override final;

	virtual bool setPartyMetaValue(uint64_t partyHandle, const std::wstring& key, const std::wstring& value) override final;

	virtual bool getPartyMetaValue(uint64_t partyHandle, const std::wstring& key, std::wstring& outValue) override final;

	virtual bool setPartyParticipantMetaValue(uint64_t partyHandle, const std::wstring& key, const std::wstring& value) override final;

	virtual bool getPartyParticipantMetaValue(uint64_t partyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue) override final;

	virtual bool getPartyParticipants(uint64_t partyHandle, std::vector< uint64_t >& outUserHandles) override final;

	virtual bool getPartyParticipantCount(uint64_t partyHandle, uint32_t& outCount) const override final;

	virtual bool inviteToParty(uint64_t partyHandle, uint64_t userHandle) override final;

private:
	std::map< std::wstring, std::wstring > m_lobbyMeta;
};

	}
}

