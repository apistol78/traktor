#ifndef traktor_online_SessionSteam_H
#define traktor_online_SessionSteam_H

#include "Online/ISession.h"

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

class T_DLLCLASS SessionSteam : public ISession
{
	T_RTTI_CLASS;

public:
	SessionSteam(CurrentUserSteam* user);

	virtual void destroy();

	virtual bool isConnected() const;

	virtual Ref< IUser > getUser();

	virtual bool getAvailableAchievements(RefArray< IAchievement >& outAchievements) const;

	virtual Ref< ISaveGame > createSaveGame(const std::wstring& name, ISerializable* attachment);

	virtual bool getAvailableSaveGames(RefArray< ISaveGame >& outSaveGames) const;

private:
	Ref< CurrentUserSteam > m_user;
};

	}
}

#endif	// traktor_online_SessionSteam_H
