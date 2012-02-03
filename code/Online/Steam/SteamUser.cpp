#include "Core/Misc/TString.h"
#include "Online/Steam/SteamUser.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SteamUser", SteamUser, IUserProvider)

bool SteamUser::getName(uint64_t userHandle, std::wstring& outName)
{
	CSteamID id(userHandle);
	if (!id.IsValid())
		return false;

	const char* name = SteamFriends()->GetFriendPersonaName(id);
	if (!name)
		return false;

	outName = mbstows(name);
	return true;
}

bool SteamUser::setPresenceValue(uint64_t userHandle, const std::wstring& key, const std::wstring& value)
{
	CSteamID id(userHandle);
	if (!id.IsValid())
		return false;

	return SteamFriends()->SetRichPresence(wstombs(key).c_str(), wstombs(value).c_str());
}

bool SteamUser::getPresenceValue(uint64_t userHandle, const std::wstring& key, std::wstring& outValue)
{
	CSteamID id(userHandle);
	if (!id.IsValid())
		return false;

	const char* value = SteamFriends()->GetFriendRichPresence(id, wstombs(key).c_str());
	if (!value)
		return false;

	outValue = mbstows(value);
	return true;
}

bool SteamUser::sendP2PData(uint64_t userHandle, const void* data, size_t size, bool reliable)
{
	CSteamID id(userHandle);
	if (!id.IsValid())
		return false;

	return SteamNetworking()->SendP2PPacket(id, data, uint32(size), reliable ? k_EP2PSendReliable : k_EP2PSendUnreliableNoDelay);
}

	}
}
