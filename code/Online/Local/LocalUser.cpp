#include "Core/System/OS.h"
#include "Online/Local/LocalUser.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LocalUser", LocalUser, IUserProvider)

bool LocalUser::getName(uint64_t userHandle, std::wstring& outName)
{
	outName = OS::getInstance().getCurrentUser();
	return true;
}

Ref< drawing::Image > LocalUser::getImage(uint64_t userHandle) const
{
	return 0;
}

bool LocalUser::isFriend(uint64_t userHandle)
{
	return true;
}

bool LocalUser::isMemberOfGroup(uint64_t userHandle, const std::wstring& groupName) const
{
	return false;
}

bool LocalUser::invite(uint64_t userHandle)
{
	return false;
}

bool LocalUser::setPresenceValue(uint64_t userHandle, const std::wstring& key, const std::wstring& value)
{
	return false;
}

bool LocalUser::getPresenceValue(uint64_t userHandle, const std::wstring& key, std::wstring& outValue)
{
	return false;
}

bool LocalUser::isP2PAllowed(uint64_t userHandle) const
{
	return false;
}

bool LocalUser::isP2PRelayed(uint64_t userHandle) const
{
	return false;
}

bool LocalUser::sendP2PData(uint64_t userHandle, const void* data, size_t size, bool reliable)
{
	return false;
}

	}
}
