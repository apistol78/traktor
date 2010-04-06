#ifndef traktor_online_SessionManagerPsn_H
#define traktor_online_SessionManagerPsn_H

#include "Online/ISessionManager.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_PSN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

class UserPsn;

/*! \brief PlayStation Network online session manager.
 * \ingroup Online
 */
class T_DLLCLASS SessionManagerPsn : public ISessionManager
{
	T_RTTI_CLASS;

public:
	SessionManagerPsn();

	virtual bool create();

	virtual void destroy();

	virtual bool getAvailableUsers(RefArray< IUser >& outUsers);

	virtual Ref< IUser > getCurrentUser();

	virtual Ref< ISession > createSession(IUser* user);

	virtual bool update();

private:
	Ref< UserPsn > m_user;
};

	}
}

#endif	// traktor_online_SessionManagerPsn_H
