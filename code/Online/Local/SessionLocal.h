#ifndef traktor_online_SessionLocal_H
#define traktor_online_SessionLocal_H

#include "Online/ISession.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_LOCAL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace sql
	{

class IConnection;

	}

	namespace online
	{

class UserLocal;

class T_DLLCLASS SessionLocal : public ISession
{
	T_RTTI_CLASS;

public:
	SessionLocal(sql::IConnection* db, UserLocal* user);

	virtual void destroy();

	virtual bool isConnected() const;

	virtual Ref< IUser > getUser();

	virtual bool getAvailableAchievements(RefArray< IAchievement >& outAchievements) const;

	virtual Ref< ISaveGame > createSaveGame(const std::wstring& name, ISerializable* attachment);

	virtual bool getAvailableSaveGames(RefArray< ISaveGame >& outSaveGames) const;

private:
	Ref< sql::IConnection > m_db;
	Ref< UserLocal > m_user;
};

	}
}

#endif	// traktor_online_SessionLocal_H
