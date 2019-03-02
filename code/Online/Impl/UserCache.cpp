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

void UserCache::getMany(const std::vector< uint64_t >& userHandles, RefArray< User >& outUsers)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	outUsers.reserve(userHandles.size());
	outUsers.resize(0);

	for (std::vector< uint64_t >::const_iterator i = userHandles.begin(); i != userHandles.end(); ++i)
	{
		SmallMap< uint64_t, Ref< User > >::const_iterator it = m_users.find(*i);
		if (it != m_users.end())
			outUsers.push_back(it->second);
		else
		{
			Ref< User > user = new User(m_userProvider, *i);
			m_users.insert(std::make_pair(*i, user));
			outUsers.push_back(user);
		}
	}
}

	}
}
