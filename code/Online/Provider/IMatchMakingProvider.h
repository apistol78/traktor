#ifndef traktor_online_IMatchMakingProvider_H
#define traktor_online_IMatchMakingProvider_H

#include <vector>
#include "Core/Object.h"

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
	virtual bool findMatchingLobbies(const LobbyFilter* filter, std::vector< uint64_t >& outLobbyHandles) = 0;

	virtual bool createLobby(uint32_t maxUsers, uint64_t& outLobbyHandle) = 0;

	virtual bool acceptLobby(uint64_t& outLobbyHandle) = 0;

	virtual bool joinLobby(uint64_t lobbyHandle) = 0;

	virtual bool leaveLobby(uint64_t lobbyHandle) = 0;

	virtual bool setMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value) = 0;

	virtual bool getMetaValue(uint64_t lobbyHandle, const std::wstring& key, std::wstring& outValue) = 0;

	virtual bool setParticipantMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value) = 0;

	virtual bool getParticipantMetaValue(uint64_t lobbyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue) = 0;

	virtual bool getParticipants(uint64_t lobbyHandle, std::vector< uint64_t >& outUserHandles) = 0;

	virtual bool getParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const = 0;

	virtual bool getMaxParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const = 0;

	virtual bool getFriendsCount(uint64_t lobbyHandle, uint32_t& outCount) const = 0;

	virtual bool invite(uint64_t lobbyHandle, uint64_t userHandle) = 0;

	virtual bool getIndex(uint64_t lobbyHandle, int32_t& outIndex) const = 0;

	virtual bool setOwner(uint64_t lobbyHandle, uint64_t userHandle) const = 0;

	virtual bool getOwner(uint64_t lobbyHandle, uint64_t& outUserHandle) const = 0;
};

	}
}

#endif	// traktor_online_IMatchMakingProvider_H
