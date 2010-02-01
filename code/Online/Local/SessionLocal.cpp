#include "Online/Local/AchievementLocal.h"
#include "Online/Local/SessionLocal.h"
#include "Online/Local/UserLocal.h"
#include "Sql/IConnection.h"
#include "Sql/IResultSet.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SessionLocal", SessionLocal, ISession)

SessionLocal::SessionLocal(sql::IConnection* db, UserLocal* user)
:	m_db(db)
,	m_user(user)
{
}

void SessionLocal::destroy()
{
}

bool SessionLocal::isConnected() const
{
	return true;
}

Ref< IUser > SessionLocal::getUser()
{
	return m_user;
}

bool SessionLocal::getAvailableAchievements(RefArray< IAchievement >& outAchievements) const
{
	Ref< sql::IResultSet > rs = m_db->executeQuery(L"select id from Achievements");
	if (!rs)
		return false;

	while (rs->next())
		outAchievements.push_back(new AchievementLocal(m_db, rs->getInt32(0)));

	return true;
}

Ref< ISaveGame > SessionLocal::createSaveGame(const std::wstring& name, ISerializable* attachment)
{
	return 0;
}

bool SessionLocal::getAvailableSaveGames(RefArray< ISaveGame >& outSaveGames) const
{
	return false;
}

	}
}
