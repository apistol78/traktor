#ifndef traktor_online_SteamUser_H
#define traktor_online_SteamUser_H

#include <set>
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
	SteamUser();

	virtual bool getName(uint64_t userHandle, std::wstring& outName);

	virtual Ref< drawing::Image > getImage(uint64_t userHandle) const;

	virtual bool isFriend(uint64_t userHandle);

	virtual bool invite(uint64_t userHandle);

	virtual bool setPresenceValue(uint64_t userHandle, const std::wstring& key, const std::wstring& value);

	virtual bool getPresenceValue(uint64_t userHandle, const std::wstring& key, std::wstring& outValue);

	virtual bool isP2PAllowed(uint64_t userHandle) const;

	virtual bool isP2PRelayed(uint64_t userHandle) const;

	virtual bool sendP2PData(uint64_t userHandle, const void* data, size_t size);

	void receivedP2PData(uint64_t userHandle);

private:
	std::set< uint64_t > m_failing;

	STEAM_CALLBACK(SteamUser, OnP2PSessionConnectFail, P2PSessionConnectFail_t, m_callbackSessionConnectFail);
};

	}
}

#endif	// traktor_online_SteamUser_H
