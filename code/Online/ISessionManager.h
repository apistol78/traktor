#ifndef traktor_online_ISessionManager_H
#define traktor_online_ISessionManager_H

#include "Core/Object.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

class IUser;
class ISession;

/*! \brief Online session manager.
 * \ingroup Online
 */
class T_DLLCLASS ISessionManager : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Create session manager.
	 *
	 * \return True if successfully created.
	 */
	virtual bool create() = 0;

	/*! \brief Destroy session manager. */
	virtual void destroy() = 0;

	/*! \brief Get available "playable" users.
	 *
	 * \param outUser Array of users.
	 * \return True if successfully accessed users.
	 */
	virtual bool getAvailableUsers(RefArray< IUser >& outUsers) = 0;

	/*! \brief Get currently logged in user.
	 *
	 * \return Logged in user; null if no user logged in.
	 */
	virtual Ref< IUser > getCurrentUser() = 0;

	/*! \brief Create play session.
	 *
	 * \param user User which will play the session.
	 * \return New play session.
	 */
	virtual Ref< ISession > createSession(IUser* user) = 0;

	/*! \brief Update back-end messaging system.
	 *
	 * Should be called frequently in order to let
	 * some poll-based systems to update their inner
	 * workings.
	 *
	 * \return True if successful.
	 */
	virtual bool update() = 0;
};

	}
}

#endif	// traktor_online_ISessionManager_H
