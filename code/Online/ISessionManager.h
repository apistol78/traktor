#ifndef traktor_online_ISessionManager_H
#define traktor_online_ISessionManager_H

#include "Core/Object.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

class IUser;
class ISession;

class T_DLLCLASS ISessionManager : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool getAvailableUsers(RefArray< IUser >& outUsers) = 0;

	virtual Ref< IUser > getCurrentUser() = 0;

	virtual Ref< ISession > createSession(IUser* user) = 0;
};

	}
}

#endif	// traktor_online_ISessionManager_H
