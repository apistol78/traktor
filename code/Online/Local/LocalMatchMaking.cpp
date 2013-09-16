#include "Online/Local/LocalMatchMaking.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LocalMatchMaking", LocalMatchMaking, IMatchMakingProvider)

bool LocalMatchMaking::findMatchingLobbies(const LobbyFilter* filter, std::vector< uint64_t >& outLobbyHandles)
{
	return false;
}

bool LocalMatchMaking::createLobby(uint32_t maxUsers, uint64_t& outLobbyHandle)
{
	outLobbyHandle = 0;
	return true;
}

bool LocalMatchMaking::acceptLobby(uint64_t& outLobbyHandle)
{
	outLobbyHandle = 0;
	return false;
}

bool LocalMatchMaking::joinLobby(uint64_t lobbyHandle)
{
	return false;
}

bool LocalMatchMaking::leaveLobby(uint64_t lobbyHandle)
{
	return true;
}

bool LocalMatchMaking::setMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value)
{
	m_lobbyMeta[key] = value;
	return true;
}

bool LocalMatchMaking::getMetaValue(uint64_t lobbyHandle, const std::wstring& key, std::wstring& outValue)
{
	std::map< std::wstring, std::wstring >::const_iterator i = m_lobbyMeta.find(key);
	if (i == m_lobbyMeta.end())
		return false;

	outValue = i->second;
	return true;
}

bool LocalMatchMaking::setParticipantMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value)
{
	return true;
}

bool LocalMatchMaking::getParticipantMetaValue(uint64_t lobbyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue)
{
	return true;
}

bool LocalMatchMaking::getParticipants(uint64_t lobbyHandle, std::vector< uint64_t >& outUserHandles)
{
	return true;
}

bool LocalMatchMaking::getParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const
{
	return false;
}

bool LocalMatchMaking::getMaxParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const
{
	return false;
}

bool LocalMatchMaking::getFriendsCount(uint64_t lobbyHandle, uint32_t& outCount) const
{
	return false;
}

bool LocalMatchMaking::invite(uint64_t lobbyHandle, uint64_t userHandle)
{
	return false;
}

bool LocalMatchMaking::getIndex(uint64_t lobbyHandle, int32_t& outIndex) const
{
	outIndex = 0;
	return true;
}

bool LocalMatchMaking::setOwner(uint64_t lobbyHandle, uint64_t userHandle) const
{
	return false;
}

bool LocalMatchMaking::getOwner(uint64_t lobbyHandle, uint64_t& outUserHandle) const
{
	outUserHandle = 0;
	return true;
}

	}
}
