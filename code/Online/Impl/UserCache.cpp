#include "Core/Thread/Acquire.h"
#include "Online/Impl/User.h"
#include "Online/Impl/UserCache.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.UserCache", UserCache, Object)

UserCache::UserCache(IUserProvider* userProvider)
:	m_userProvider(userProvider)
{
}

User* UserCache::get(uint64_t userHandle)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	SmallMap< uint64_t, Ref< User > >::const_iterator i = m_users.find(userHandle);
	if (i != m_users.end())
		return i->second;

	Ref< User > user = new User(m_userProvider, userHandle);
	m_users.insert(std::make_pair(userHandle, user));

	return user;
}

	}
}
