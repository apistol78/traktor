#ifndef traktor_online_SessionManagerSteam_H
#define traktor_online_SessionManagerSteam_H

#include "Online/ISessionManager.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_STEAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

class CurrentUserSteam;
class SessionSteam;

class T_DLLCLASS SessionManagerSteam : public ISessionManager
{
	T_RTTI_CLASS;

public:
	SessionManagerSteam();

	virtual bool create();

	virtual void destroy();

	virtual std::wstring getLanguageCode() const;

	virtual bool getAvailableUsers(RefArray< IUser >& outUsers);

	virtual Ref< IUser > getCurrentUser();

	virtual Ref< ISession > createSession(IUser* user, const std::set< std::wstring >& leaderboards);

	virtual bool requireUserAttention() const;

	virtual bool update();

private:
	Ref< CurrentUserSteam > m_currentUser;
	Ref< SessionSteam > m_session;
	bool m_userAttention;
	
	STEAM_CALLBACK(SessionManagerSteam, OnOverlayActivated, GameOverlayActivated_t, m_callbackOverlay);
};

	}
}

#endif	// traktor_online_SessionManagerSteam_H
