#ifndef traktor_online_SessionLocal_H
#define traktor_online_SessionLocal_H

#include "Online/ISession.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_LOCAL_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

class UserLocal;

class T_DLLCLASS SessionLocal : public ISession
{
	T_RTTI_CLASS(SessionLocal)

public:
	SessionLocal(UserLocal* user);

	virtual void destroy();

	virtual bool isConnected() const;

	virtual Ref< IUser > getUser();

	virtual bool getAvailableAchievements(RefArray< IAchievement >& outAchievements) const;

	virtual Ref< ISaveGame > createSaveGame(const std::wstring& name, Serializable* attachment);

	virtual bool getAvailableSaveGames(RefArray< ISaveGame >& outSaveGames) const;

private:
	Ref< UserLocal > m_user;
};

	}
}

#endif	// traktor_online_SessionLocal_H
