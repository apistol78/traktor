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
	CSteamID id(userHandle);
	if (!id.IsValid())
		return false;

	const char* name = SteamFriends()->GetFriendPersonaName(id);
	if (!name)
		return false;

	outName = mbstows(Utf8Encoding(), name);
	return true;
}

Ref< drawing::Image > SteamUser::getImage(uint64_t userHandle) const
{
	CSteamID id(userHandle);
	if (!id.IsValid())
		return 0;

	int handle = SteamFriends()->GetMediumFriendAvatar(id);
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
	CSteamID id(userHandle);
	if (!id.IsValid())
		return false;

	return SteamFriends()->GetFriendRelationship(id) == k_EFriendRelationshipFriend;
}

bool SteamUser::invite(uint64_t userHandle)
{
	CSteamID id(userHandle);
	if (!id.IsValid())
		return false;

	return SteamFriends()->InviteUserToGame(id, "");
}

bool SteamUser::setPresenceValue(uint64_t userHandle, const std::wstring& key, const std::wstring& value)
{
	CSteamID id(userHandle);
	if (!id.IsValid())
		return false;

	return SteamFriends()->SetRichPresence(wstombs(key).c_str(), wstombs(Utf8Encoding(), value).c_str());
}

bool SteamUser::getPresenceValue(uint64_t userHandle, const std::wstring& key, std::wstring& outValue)
{
	CSteamID id(userHandle);
	if (!id.IsValid())
		return false;

	const char* value = SteamFriends()->GetFriendRichPresence(id, wstombs(key).c_str());
	if (!value)
		return false;

	outValue = mbstows(Utf8Encoding(), value);
	return true;
}

bool SteamUser::isP2PAllowed(uint64_t userHandle) const
{
	return m_failing.find(userHandle) == m_failing.end();
}

bool SteamUser::sendP2PData(uint64_t userHandle, const void* data, size_t size)
{
	if (m_failing.find(userHandle) != m_failing.end())
		return false;

	CSteamID id(userHandle);
	if (!id.IsValid())
		return false;

	return SteamNetworking()->SendP2PPacket(id, data, uint32(size), k_EP2PSendUnreliableNoDelay);
}

void SteamUser::receivedP2PData(uint64_t userHandle)
{
	if (m_failing.erase(userHandle) > 0)
		log::info << L"Steam; P2P session recovered peer " << userHandle << Endl;
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
	if (m_failing.insert(userHandle).second)
	{
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
}
