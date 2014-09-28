#ifndef traktor_online_UserCache_H
#define traktor_online_UserCache_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{
	namespace online
	{

class IUserProvider;
class User;

class UserCache : public Object
{
	T_RTTI_CLASS;

public:
	UserCache(IUserProvider* userProvider);

	User* get(uint64_t userHandle);

	void getMany(const std::vector< uint64_t >& userHandles, RefArray< User >& outUsers);

private:
	Semaphore m_lock;
	Ref< IUserProvider > m_userProvider;
	SmallMap< uint64_t, Ref< User > > m_users;
};

	}
}

#endif	// traktor_online_UserCache_H
