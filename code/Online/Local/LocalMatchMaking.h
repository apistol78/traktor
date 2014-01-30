#ifndef traktor_online_LocalMatchMaking_H
#define traktor_online_LocalMatchMaking_H

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

private:
	std::map< std::wstring, std::wstring > m_lobbyMeta;
};

	}
}

#endif	// traktor_online_LocalMatchMaking_H
