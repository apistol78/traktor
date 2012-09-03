#ifndef traktor_online_UserCache_H
#define traktor_online_UserCache_H

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"

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

private:
	Ref< IUserProvider > m_userProvider;
	SmallMap< uint64_t, Ref< User > > m_users;
	int32_t m_nextTag;
};

	}
}

#endif	// traktor_online_UserCache_H
