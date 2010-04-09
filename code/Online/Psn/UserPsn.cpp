#include "Online/Psn/UserPsn.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.UserPsn", UserPsn, IUser)

UserPsn::UserPsn(const std::wstring& name)
:	m_name(name)
{
}

std::wstring UserPsn::getName() const
{
	return m_name;
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
