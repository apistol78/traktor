#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Online/Steam/SteamUser.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SteamUser", SteamUser, IUserProvider)

SteamUser::SteamUser()
:	m_callbackSessionConnectFail(this, &SteamUser::OnP2PSessionConnectFail)
{
}

bool SteamUser::getName(uint64_t userHandle, std::wstring& outName)
{
	const char* name = SteamFriends()->GetFriendPersonaName(uint64(userHandle));
	if (!name)
		return false;

	outName = mbstows(Utf8Encoding(), name);
	return true;
}

Ref< drawing::Image > SteamUser::getImage(uint64_t userHandle) const
{
	int handle = SteamFriends()->GetMediumFriendAvatar(uint64(userHandle));
	if (!handle)
		return 0;

	Ref< drawing::Image > image = new drawing::Image(drawing::PixelFormat::getA8B8G8R8(), 64, 64);

	if (!SteamUtils()->GetImageRGBA(
		handle,
		(uint8*)image->getData(),
		image->getDataSize()
	))
		return 0;

	return image;
}

bool SteamUser::isFriend(uint64_t userHandle)
{
	return SteamFriends()->GetFriendRelationship(uint64(userHandle)) == k_EFriendRelationshipFriend;
}

bool SteamUser::invite(uint64_t userHandle)
{
	return SteamFriends()->InviteUserToGame(uint64(userHandle), "");
}

bool SteamUser::setPresenceValue(uint64_t userHandle, const std::wstring& key, const std::wstring& value)
{
	return SteamFriends()->SetRichPresence(wstombs(key).c_str(), wstombs(Utf8Encoding(), value).c_str());
}

bool SteamUser::getPresenceValue(uint64_t userHandle, const std::wstring& key, std::wstring& outValue)
{
	const char* value = SteamFriends()->GetFriendRichPresence(uint64(userHandle), wstombs(key).c_str());
	if (!value)
		return false;

	outValue = mbstows(Utf8Encoding(), value);
	return true;
}

bool SteamUser::isP2PAllowed(uint64_t userHandle) const
{
	return true;
}

bool SteamUser::isP2PRelayed(uint64_t userHandle) const
{
	P2PSessionState_t ss;
	if (!SteamNetworking()->GetP2PSessionState(uint64(userHandle), &ss))
		return false;

	return ss.m_bUsingRelay != 0;
}

bool SteamUser::sendP2PData(uint64_t userHandle, const void* data, size_t size)
{
	bool result = false;
	T_EXCEPTION_GUARD_BEGIN
	
	if (data && size > 0)
		result = SteamNetworking()->SendP2PPacket(uint64(userHandle), data, uint32(size), k_EP2PSendUnreliableNoDelay);
	else
		result = true;

	T_EXCEPTION_GUARD_END
	return result;
}

void SteamUser::OnP2PSessionConnectFail(P2PSessionConnectFail_t* pP2PSessionConnectFail)
{
	const wchar_t* hr[] =
	{
		L"k_EP2PSessionErrorNone",
		L"k_EP2PSessionErrorNotRunningApp",
		L"k_EP2PSessionErrorNoRightsToApp",
		L"k_EP2PSessionErrorDestinationNotLoggedIn",
		L"k_EP2PSessionErrorTimeout"
	};

	uint64_t userHandle = uint64_t(pP2PSessionConnectFail->m_steamIDRemote.ConvertToUint64());
	const char* name = SteamFriends()->GetFriendPersonaName(pP2PSessionConnectFail->m_steamIDRemote);
	if (name)
	{
		std::wstring wname = mbstows(Utf8Encoding(), name);
		log::error << L"Steam; P2P session connect fail to peer \"" << wname << L"\", m_eP2PSessionError = " << int32_t(pP2PSessionConnectFail->m_eP2PSessionError) << L" (" << hr[pP2PSessionConnectFail->m_eP2PSessionError] << L")" << Endl;
	}
	else
		log::error << L"Steam; P2P session connect fail to peer " << userHandle << L", m_eP2PSessionError = " << int32_t(pP2PSessionConnectFail->m_eP2PSessionError) << L" (" << hr[pP2PSessionConnectFail->m_eP2PSessionError] << L")" << Endl;
}

	}
}
