#include <np.h>
#include "Core/Io/Utf8Encoding.h"
#include "Core/Misc/TString.h"
#include "Online/Psn/UserPsn.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.UserPsn", UserPsn, IUser)

UserPsn::UserPsn()
{
}

std::wstring UserPsn::getName() const
{
	SceNpOnlineName onlineName;
	if (sceNpManagerGetOnlineName(&onlineName) < 0)
		return L"";
	return mbstows(Utf8Encoding(), onlineName.data);
}

bool UserPsn::getFriends(RefArray< IUser >& outFriends) const
{
	return true;
}

bool UserPsn::sendMessage(const std::wstring& message) const
{
	return false;
}

	}
}
