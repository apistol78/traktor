#ifndef traktor_online_UserLocal_H
#define traktor_online_UserLocal_H

#include "Online/IUser.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_LOCAL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

class UserPsn : public IUser
{
	T_RTTI_CLASS;

public:
	UserPsn(const std::wstring& name);

	virtual std::wstring getName() const;

	virtual bool getFriends(RefArray< IUser >& outFriends) const;

	virtual bool sendMessage(const std::wstring& message) const;

private:
	std::wstring m_name;
};

	}
}

#endif	// traktor_online_UserLocal_H
