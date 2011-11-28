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

	virtual bool findMatchingLobbies(const std::wstring& key, const std::wstring& value, std::vector< LobbyData >& outLobbies);

	virtual bool createLobby(uint32_t maxUsers, LobbyData& outLobby);

	virtual bool joinLobby(uint64_t lobbyHandle);

	virtual bool setMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value);

private:
	SteamSessionManager* m_sessionManager;
	std::vector< LobbyData >* m_outLobbies;
	LobbyData* m_outLobby;
	bool m_joinResult;
	CCallResult< SteamMatchMaking, LobbyMatchList_t > m_callbackLobbyMatch;
	CCallResult< SteamMatchMaking, LobbyCreated_t > m_callbackLobbyCreated;
	CCallResult< SteamMatchMaking, LobbyEnter_t > m_callbackLobbyEnter;

	void OnLobbyMatch(LobbyMatchList_t* pCallback, bool bIOFailure);

	void OnLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure);

	void OnLobbyEnter(LobbyEnter_t* pCallback, bool bIOFailure);
};

	}
}

#endif	// traktor_online_SteamMatchMaking_H
