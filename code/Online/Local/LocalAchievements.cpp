/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Online/Local/LocalAchievements.h"
#include "Sql/IConnection.h"
#include "Sql/IResultSet.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LocalAchievements", LocalAchievements, IAchievementsProvider)

LocalAchievements::LocalAchievements(sql::IConnection* db)
:	m_db(db)
{
}

bool LocalAchievements::enumerate(std::map< std::wstring, bool >& outAchievements)
{
	Ref< sql::IResultSet > rs;

	rs = m_db->executeQuery(L"select * from Achievements");
	if (!rs)
		return false;

	while (rs->next())
	{
		outAchievements.insert(std::make_pair(
			rs->getString(L"id"),
			rs->getInt32(L"reward") != 0
		));
	}

	return true;
}

bool LocalAchievements::set(const std::wstring& achievementId, bool reward)
{
	if (m_db->executeUpdate(L"update Achievements set reward=" + std::wstring(reward ? L"1" : L"0") + L" where id='" + achievementId + L"'") > 0)
		return true;
	else
		return false;
}

	}
}
