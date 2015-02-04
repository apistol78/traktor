#include "Online/Gc/GcMatchMaking.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.GcMatchMaking", GcMatchMaking, IMatchMakingProvider)

bool GcMatchMaking::findMatchingLobbies(const LobbyFilter* filter, std::vector< uint64_t >& outLobbyHandles)
{
	return false;
}

bool GcMatchMaking::createLobby(uint32_t maxUsers, LobbyAccess access, uint64_t& outLobbyHandle)
{
	outLobbyHandle = 0;
	return true;
}

bool GcMatchMaking::acceptLobby(uint64_t& outLobbyHandle)
{
	outLobbyHandle = 0;
	return false;
}

bool GcMatchMaking::joinLobby(uint64_t lobbyHandle)
{
	return false;
}

bool GcMatchMaking::leaveLobby(uint64_t lobbyHandle)
{
	return true;
}

bool GcMatchMaking::setLobbyMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value)
{
	m_lobbyMeta[key] = value;
	return true;
}

bool GcMatchMaking::getLobbyMetaValue(uint64_t lobbyHandle, const std::wstring& key, std::wstring& outValue)
{
	std::map< std::wstring, std::wstring >::const_iterator i = m_lobbyMeta.find(key);
	if (i == m_lobbyMeta.end())
		return false;

	outValue = i->second;
	return true;
}

bool GcMatchMaking::setLobbyParticipantMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value)
{
	return true;
}

bool GcMatchMaking::getLobbyParticipantMetaValue(uint64_t lobbyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue)
{
	return true;
}

bool GcMatchMaking::getLobbyParticipants(uint64_t lobbyHandle, std::vector< uint64_t >& outUserHandles)
{
	return true;
}

bool GcMatchMaking::getLobbyParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const
{
	return false;
}

bool GcMatchMaking::getLobbyMaxParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const
{
	return false;
}

bool GcMatchMaking::getLobbyFriendsCount(uint64_t lobbyHandle, uint32_t& outCount) const
{
	return false;
}

bool GcMatchMaking::inviteToLobby(uint64_t lobbyHandle, uint64_t userHandle)
{
	return false;
}

bool GcMatchMaking::setLobbyOwner(uint64_t lobbyHandle, uint64_t userHandle) const
{
	return false;
}

bool GcMatchMaking::getLobbyOwner(uint64_t lobbyHandle, uint64_t& outUserHandle) const
{
	outUserHandle = 0;
	return true;
}

bool GcMatchMaking::createParty(uint64_t& outPartyHandle)
{
	return false;
}

bool GcMatchMaking::acceptParty(uint64_t& outPartyHandle)
{
	return false;
}

bool GcMatchMaking::joinParty(uint64_t partyHandle)
{
	return false;
}

bool GcMatchMaking::leaveParty(uint64_t partyHandle)
{
	return false;
}

bool GcMatchMaking::setPartyMetaValue(uint64_t partyHandle, const std::wstring& key, const std::wstring& value)
{
	return false;
}

bool GcMatchMaking::getPartyMetaValue(uint64_t partyHandle, const std::wstring& key, std::wstring& outValue)
{
	return false;
}

bool GcMatchMaking::setPartyParticipantMetaValue(uint64_t partyHandle, const std::wstring& key, const std::wstring& value)
{
	return false;
}

bool GcMatchMaking::getPartyParticipantMetaValue(uint64_t partyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue)
{
	return false;
}

bool GcMatchMaking::getPartyParticipants(uint64_t partyHandle, std::vector< uint64_t >& outUserHandles)
{
	return false;
}

bool GcMatchMaking::getPartyParticipantCount(uint64_t partyHandle, uint32_t& outCount) const
{
	return false;
}

bool GcMatchMaking::inviteToParty(uint64_t partyHandle, uint64_t userHandle)
{
	return false;
}

	}
}
