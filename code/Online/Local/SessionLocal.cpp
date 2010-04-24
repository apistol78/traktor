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

bool SessionLocal::rewardAchievement(const std::wstring& achievementId)
{
	Ref< sql::IResultSet > rs;

	rs = m_db->executeQuery(L"select id from Achievements where name='" + achievementId + L"'");
	if (!rs || !rs->next())
		return false;

	int32_t aid = rs->getInt32(0);
	int32_t uid = m_user->getId();

	return m_db->executeUpdate(L"insert into AwardedAchievements (achievementId, userId) values (" + toString(aid) + L", " + toString(uid) + L")") >= 0;
}

bool SessionLocal::withdrawAchievement(const std::wstring& achievementId)
{
	Ref< sql::IResultSet > rs;

	rs = m_db->executeQuery(L"select id from Achievements where name='" + achievementId + L"'");
	if (!rs || !rs->next())
		return false;

	int32_t aid = rs->getInt32(0);
	int32_t uid = m_user->getId();

	return m_db->executeUpdate(L"delete from AwardedAchievements where userId=" + toString(uid) + L" and achievementId=" + toString(aid)) >= 0;
}

bool SessionLocal::setStatValue(const std::wstring& statId, float value)
{
	if (m_db->executeUpdate(L"update Stats set value=" + toString(value) + L" where name='" + statId + L"'"))
		return true;

	return m_db->executeUpdate(L"insert into Stats (name, value) values ('" + statId + L"', " + toString(value) + L")") >= 0;
}

bool SessionLocal::getStatValue(const std::wstring& statId, float& outValue)
{
	Ref< sql::IResultSet > rs;

	rs = m_db->executeQuery(L"select value from Stats where name='" + statId + L"'");
	if (!rs || !rs->next())
		return false;

	outValue = rs->getFloat(0);
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
