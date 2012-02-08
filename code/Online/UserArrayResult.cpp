#include "Online/UserArrayResult.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.UserArrayResult", UserArrayResult, Result)

void UserArrayResult::succeed(const RefArray< IUser >& users)
{
	m_users = users;
	Result::succeed();
}

const RefArray< IUser >& UserArrayResult::get() const
{
	wait();
	return m_users;
}

	}
}
