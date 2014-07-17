#ifndef traktor_online_SteamMatchMaking_H
#define traktor_online_SteamMatchMaking_H

#include <steam/steam_api.h>
#include "Online/Provider/IMatchMakingProvider.h"

namespace traktor
{
	namespace online
	{

class SteamSessionManager;

class SteamMatchMaking : public IMatchMakingProvider
{
	T_RTTI_CLASS;

public:
	SteamMatchMaking(SteamSessionManager* sessionManager);

	virtual bool findMatchingLobbies(const LobbyFilter* filter, std::vector< uint64_t >& outLobbyHandles);

	virtual bool createLobby(uint32_t maxUsers, LobbyAccess access, uint64_t& outLobbyHandle);

	virtual bool acceptLobby(uint64_t& outLobbyHandle);

	virtual bool joinLobby(uint64_t lobbyHandle);

	virtual bool leaveLobby(uint64_t lobbyHandle);

	virtual bool setMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value);

	virtual bool getMetaValue(uint64_t lobbyHandle, const std::wstring& key, std::wstring& outValue);

	virtual bool setParticipantMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value);

	virtual bool getParticipantMetaValue(uint64_t lobbyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue);

	virtual bool getParticipants(uint64_t lobbyHandle, std::vector< uint64_t >& outUserHandles);

	virtual bool getParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const;

	virtual bool getMaxParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const;

	virtual bool getFriendsCount(uint64_t lobbyHandle, uint32_t& outCount) const;

	virtual bool invite(uint64_t lobbyHandle, uint64_t userHandle);

	virtual bool getIndex(uint64_t lobbyHandle, int32_t& outIndex) const;

	virtual bool setOwner(uint64_t lobbyHandle, uint64_t userHandle) const;

	virtual bool getOwner(uint64_t lobbyHandle, uint64_t& outUserHandle) const;

	void update();

private:
	SteamSessionManager* m_sessionManager;
	std::vector< uint64_t >* m_outLobbies;
	uint64_t* m_outLobby;
	uint64_t m_acceptedInvite;
	uint64_t m_joinedLobby;
	bool m_joinResult;
	CCallResult< SteamMatchMaking, LobbyMatchList_t > m_callbackLobbyMatch;
	CCallResult< SteamMatchMaking, LobbyCreated_t > m_callbackLobbyCreated;
	CCallResult< SteamMatchMaking, LobbyEnter_t > m_callbackLobbyEnter;

	void OnLobbyMatch(LobbyMatchList_t* pCallback, bool bIOFailure);

	void OnLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure);

	void OnLobbyEnter(LobbyEnter_t* pCallback, bool bIOFailure);

	STEAM_CALLBACK(SteamMatchMaking, OnGameLobbyJoinRequested, GameLobbyJoinRequested_t, m_callbackGameLobbyJoinRequested);
};

	}
}

#endif	// traktor_online_SteamMatchMaking_H
