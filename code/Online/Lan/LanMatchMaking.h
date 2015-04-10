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
	LanMatchMaking(net::DiscoveryManager* discoveryManager, uint64_t userHandle);

	void update();

	// Lobby

	virtual bool findMatchingLobbies(const LobbyFilter* filter, std::vector< uint64_t >& outLobbyHandles);

	virtual bool createLobby(uint32_t maxUsers, LobbyAccess access, uint64_t& outLobbyHandle);

	virtual bool acceptLobby(uint64_t& outLobbyHandle);

	virtual bool joinLobby(uint64_t lobbyHandle);

	virtual bool leaveLobby(uint64_t lobbyHandle);

	virtual bool setLobbyMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value);

	virtual bool getLobbyMetaValue(uint64_t lobbyHandle, const std::wstring& key, std::wstring& outValue);

	virtual bool setLobbyParticipantMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value);

	virtual bool getLobbyParticipantMetaValue(uint64_t lobbyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue);

	virtual bool getLobbyParticipants(uint64_t lobbyHandle, std::vector< uint64_t >& outUserHandles);

	virtual bool getLobbyParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const;

	virtual bool getLobbyMaxParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const;

	virtual bool getLobbyFriendsCount(uint64_t lobbyHandle, uint32_t& outCount) const;

	virtual bool inviteToLobby(uint64_t lobbyHandle, uint64_t userHandle);

	virtual bool setLobbyOwner(uint64_t lobbyHandle, uint64_t userHandle) const;

	virtual bool getLobbyOwner(uint64_t lobbyHandle, uint64_t& outUserHandle) const;

	// Party

	virtual bool createParty(uint64_t& outPartyHandle);

	virtual bool acceptParty(uint64_t& outPartyHandle);

	virtual bool joinParty(uint64_t partyHandle);

	virtual bool leaveParty(uint64_t partyHandle);

	virtual bool setPartyMetaValue(uint64_t partyHandle, const std::wstring& key, const std::wstring& value);

	virtual bool getPartyMetaValue(uint64_t partyHandle, const std::wstring& key, std::wstring& outValue);

	virtual bool setPartyParticipantMetaValue(uint64_t partyHandle, const std::wstring& key, const std::wstring& value);

	virtual bool getPartyParticipantMetaValue(uint64_t partyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue);

	virtual bool getPartyParticipants(uint64_t partyHandle, std::vector< uint64_t >& outUserHandles);

	virtual bool getPartyParticipantCount(uint64_t partyHandle, uint32_t& outCount) const;

	virtual bool inviteToParty(uint64_t partyHandle, uint64_t userHandle);

private:
	mutable Semaphore m_lock;

	Ref< net::DiscoveryManager > m_discoveryManager;
	Ref< net::NetworkService > m_userInLobbyService;

	uint64_t m_userHandle;
	uint64_t m_lobbyHandle;

	std::vector< uint64_t > m_lobbyUsers;
	uint64_t m_primaryLobbyUser;
};

	}
}

#endif	// traktor_online_LanMatchMaking_H
