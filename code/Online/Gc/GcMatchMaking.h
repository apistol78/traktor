#ifndef traktor_online_GcMatchMaking_H
#define traktor_online_GcMatchMaking_H

#include <map>
#include "Online/Provider/IMatchMakingProvider.h"

namespace traktor
{
	namespace online
	{

class GcMatchMaking : public IMatchMakingProvider
{
	T_RTTI_CLASS;

public:
	virtual bool findMatchingLobbies(const std::wstring& key, const std::wstring& value, std::vector< uint64_t >& outLobbyHandles);

	virtual bool createLobby(uint32_t maxUsers, uint64_t& outLobbyHandle);

	virtual bool joinLobby(uint64_t lobbyHandle);

	virtual bool leaveLobby(uint64_t lobbyHandle);

	virtual bool setMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value);

	virtual bool getMetaValue(uint64_t lobbyHandle, const std::wstring& key, std::wstring& outValue);

	virtual bool setParticipantMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value);

	virtual bool getParticipantMetaValue(uint64_t lobbyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue);

	virtual bool getParticipants(uint64_t lobbyHandle, std::vector< uint64_t >& outUserHandles);

	virtual bool getIndex(uint64_t lobbyHandle, int32_t& outIndex) const;

private:
	std::map< std::wstring, std::wstring > m_lobbyMeta;
};

	}
}

#endif	// traktor_online_GcMatchMaking_H
