#ifndef traktor_online_SteamUser_H
#define traktor_online_SteamUser_H

#include <steam/steam_api.h>
#include "Online/Provider/IUserProvider.h"

namespace traktor
{
	namespace online
	{

class SteamUser : public IUserProvider
{
	T_RTTI_CLASS;

public:
	virtual bool getName(uint64_t userHandle, std::wstring& outName);

	virtual bool sendP2PData(uint64_t userHandle, const void* data, size_t size, bool reliable);
};

	}
}

#endif	// traktor_online_SteamUser_H
