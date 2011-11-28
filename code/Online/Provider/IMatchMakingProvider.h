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

class T_DLLCLASS IMatchMakingProvider : public Object
{
	T_RTTI_CLASS;

public:
	struct LobbyData
	{
		uint64_t handle;
	};

	virtual bool findMatchingLobbies(const std::wstring& key, const std::wstring& value, std::vector< LobbyData >& outLobbies) = 0;

	virtual bool createLobby(uint32_t maxUsers, LobbyData& outLobby) = 0;

	virtual bool joinLobby(uint64_t lobbyHandle) = 0;

	virtual bool setMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value) = 0;
};

	}
}

#endif	// traktor_online_IMatchMakingProvider_H
