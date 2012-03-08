#include "Online/Gc/GcMatchMaking.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.GcMatchMaking", GcMatchMaking, IMatchMakingProvider)

bool GcMatchMaking::findMatchingLobbies(const std::wstring& key, const std::wstring& value, std::vector< uint64_t >& outLobbyHandles)
{
	return false;
}

bool GcMatchMaking::createLobby(uint32_t maxUsers, uint64_t& outLobbyHandle)
{
	outLobbyHandle = 0;
	return true;
}

bool GcMatchMaking::joinLobby(uint64_t lobbyHandle)
{
	return false;
}

bool GcMatchMaking::leaveLobby(uint64_t lobbyHandle)
{
	return true;
}

bool GcMatchMaking::setMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value)
{
	m_lobbyMeta[key] = value;
	return true;
}

bool GcMatchMaking::getMetaValue(uint64_t lobbyHandle, const std::wstring& key, std::wstring& outValue)
{
	std::map< std::wstring, std::wstring >::const_iterator i = m_lobbyMeta.find(key);
	if (i == m_lobbyMeta.end())
		return false;

	outValue = i->second;
	return true;
}

bool GcMatchMaking::setParticipantMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value)
{
	return true;
}

bool GcMatchMaking::getParticipantMetaValue(uint64_t lobbyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue)
{
	return true;
}

bool GcMatchMaking::getParticipants(uint64_t lobbyHandle, std::vector< uint64_t >& outUserHandles)
{
	return true;
}

bool GcMatchMaking::getIndex(uint64_t lobbyHandle, int32_t& outIndex) const
{
	outIndex = 0;
	return true;
}

	}
}
