#include "Online/Psn/SessionManagerPsn.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.online.SessionManagerPsn", 0, SessionManagerPsn, ISessionManager)

bool SessionManagerPsn::getAvailableUsers(RefArray< IUser >& outUsers)
{
	return true;
}

Ref< IUser > SessionManagerPsn::getCurrentUser()
{
	return 0;
}

Ref< ISession > SessionManagerPsn::createSession(IUser* user)
{
	return 0;
}

	}
}
