#ifndef traktor_online_CurrentUserSteam_H
#define traktor_online_CurrentUserSteam_H

#include "Online/IUser.h"

namespace traktor
{
	namespace online
	{

class CurrentUserSteam : public IUser
{
	T_RTTI_CLASS;

public:
	CurrentUserSteam();

	virtual std::wstring getName() const;

	virtual bool getFriends(RefArray< IUser >& outFriends) const;

	virtual bool sendMessage(const std::wstring& message) const;
};

	}
}

#endif	// traktor_online_CurrentUserSteam_H
