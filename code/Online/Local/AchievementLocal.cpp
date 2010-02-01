#include "Online/Local/AchievementLocal.h"
#include "Sql/IConnection.h"
#include "Sql/IResultSet.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.AchievementLocal", AchievementLocal, IAchievement)

AchievementLocal::AchievementLocal(sql::IConnection* db, int32_t id)
:	m_db(db)
,	m_id(id)
{
}

int32_t AchievementLocal::getId() const
{
	return m_id;
}

std::wstring AchievementLocal::getName() const
{
	Ref< sql::IResultSet > rs = m_db->executeQuery(L"select name from Achievements where id = " + toString(m_id));
	return (rs && rs->next()) ? rs->getString(0) : L"";
}

uint32_t AchievementLocal::getScoreRating() const
{
	Ref< sql::IResultSet > rs = m_db->executeQuery(L"select rating from Achievements where id = " + toString(m_id));
	return (rs && rs->next()) ? rs->getInt32(0) : 0;
}

	}
}
