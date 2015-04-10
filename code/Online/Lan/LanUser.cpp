#include "Core/Misc/String.h"
#include "Online/Lan/LanUser.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LocalUser", LanUser, IUserProvider)

LanUser::LanUser()
:	m_nameIndex(0)
{
}

bool LanUser::getName(uint64_t userHandle, std::wstring& outName)
{
	outName = m_nameCache[userHandle];
	if (outName.empty())
	{
		outName = std::wstring(L"User_") + toString(++m_nameIndex);
		m_nameCache[userHandle] = outName;
	}
	return true;
}

Ref< drawing::Image > LanUser::getImage(uint64_t userHandle) const
{
	return 0;
}

bool LanUser::isFriend(uint64_t userHandle)
{
	return false;
}

bool LanUser::isMemberOfGroup(uint64_t userHandle, const std::wstring& groupName) const
{
	return false;
}

bool LanUser::joinGroup(uint64_t userHandle, const std::wstring& groupName)
{
	return false;
}

bool LanUser::invite(uint64_t userHandle)
{
	return false;
}

bool LanUser::setPresenceValue(uint64_t userHandle, const std::wstring& key, const std::wstring& value)
{
	return false;
}

bool LanUser::getPresenceValue(uint64_t userHandle, const std::wstring& key, std::wstring& outValue)
{
	return false;
}

bool LanUser::isP2PAllowed(uint64_t userHandle) const
{
	return true;
}

bool LanUser::isP2PRelayed(uint64_t userHandle) const
{
	return false;
}

bool LanUser::sendP2PData(uint64_t userHandle, const void* data, size_t size, bool reliable)
{
	return false;
}

	}
}
