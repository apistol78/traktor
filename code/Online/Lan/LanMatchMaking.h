/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_LanMatchMaking_H
#define traktor_online_LanMatchMaking_H

#include "Core/Thread/Semaphore.h"
#include "Online/Provider/IMatchMakingProvider.h"

namespace traktor
{
	namespace net
	{

class DiscoveryManager;
class NetworkService;

	}

	namespace online
	{

class LanMatchMaking : public IMatchMakingProvider
{
	T_RTTI_CLASS;

public:
	LanMatchMaking(net::DiscoveryManager* discoveryManager, int32_t userHandle);

	void update();

	// Lobby

	virtual bool findMatchingLobbies(const LobbyFilter* filter, std::vector< uint64_t >& outLobbyHandles) T_OVERRIDE T_FINAL;

	virtual bool createLobby(uint32_t maxUsers, LobbyAccess access, uint64_t& outLobbyHandle) T_OVERRIDE T_FINAL;

	virtual bool acceptLobby(uint64_t& outLobbyHandle) T_OVERRIDE T_FINAL;

	virtual bool joinLobby(uint64_t lobbyHandle) T_OVERRIDE T_FINAL;

	virtual bool leaveLobby(uint64_t lobbyHandle) T_OVERRIDE T_FINAL;

	virtual bool setLobbyMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value) T_OVERRIDE T_FINAL;

	virtual bool getLobbyMetaValue(uint64_t lobbyHandle, const std::wstring& key, std::wstring& outValue) T_OVERRIDE T_FINAL;

	virtual bool setLobbyParticipantMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value) T_OVERRIDE T_FINAL;

	virtual bool getLobbyParticipantMetaValue(uint64_t lobbyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue) T_OVERRIDE T_FINAL;

	virtual bool getLobbyParticipants(uint64_t lobbyHandle, std::vector< uint64_t >& outUserHandles) T_OVERRIDE T_FINAL;

	virtual bool getLobbyParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const T_OVERRIDE T_FINAL;

	virtual bool getLobbyMaxParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const T_OVERRIDE T_FINAL;

	virtual bool getLobbyFriendsCount(uint64_t lobbyHandle, uint32_t& outCount) const T_OVERRIDE T_FINAL;

	virtual bool inviteToLobby(uint64_t lobbyHandle, uint64_t userHandle) T_OVERRIDE T_FINAL;

	virtual bool setLobbyOwner(uint64_t lobbyHandle, uint64_t userHandle) const T_OVERRIDE T_FINAL;

	virtual bool getLobbyOwner(uint64_t lobbyHandle, uint64_t& outUserHandle) const T_OVERRIDE T_FINAL;

	// Party

	virtual bool createParty(uint64_t& outPartyHandle) T_OVERRIDE T_FINAL;

	virtual bool acceptParty(uint64_t& outPartyHandle) T_OVERRIDE T_FINAL;

	virtual bool joinParty(uint64_t partyHandle) T_OVERRIDE T_FINAL;

	virtual bool leaveParty(uint64_t partyHandle) T_OVERRIDE T_FINAL;

	virtual bool setPartyMetaValue(uint64_t partyHandle, const std::wstring& key, const std::wstring& value) T_OVERRIDE T_FINAL;

	virtual bool getPartyMetaValue(uint64_t partyHandle, const std::wstring& key, std::wstring& outValue) T_OVERRIDE T_FINAL;

	virtual bool setPartyParticipantMetaValue(uint64_t partyHandle, const std::wstring& key, const std::wstring& value) T_OVERRIDE T_FINAL;

	virtual bool getPartyParticipantMetaValue(uint64_t partyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue) T_OVERRIDE T_FINAL;

	virtual bool getPartyParticipants(uint64_t partyHandle, std::vector< uint64_t >& outUserHandles) T_OVERRIDE T_FINAL;

	virtual bool getPartyParticipantCount(uint64_t partyHandle, uint32_t& outCount) const T_OVERRIDE T_FINAL;

	virtual bool inviteToParty(uint64_t partyHandle, uint64_t userHandle) T_OVERRIDE T_FINAL;

private:
	mutable Semaphore m_lock;

	Ref< net::DiscoveryManager > m_discoveryManager;
	mutable Ref< net::NetworkService > m_userInLobbyService;

	int32_t m_userHandle;
	int32_t m_lobbyHandle;

	std::vector< int32_t > m_lobbyUsers;
	uint64_t m_primaryLobbyUser;
};

	}
}

#endif	// traktor_online_LanMatchMaking_H
