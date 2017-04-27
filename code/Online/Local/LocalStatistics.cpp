/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Online/Local/LocalStatistics.h"
#include "Sql/IConnection.h"
#include "Sql/IResultSet.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LocalStatistics", LocalStatistics, IStatisticsProvider)

LocalStatistics::LocalStatistics(sql::IConnection* db)
:	m_db(db)
{
}

bool LocalStatistics::enumerate(std::map< std::wstring, int32_t >& outStats)
{
	Ref< sql::IResultSet > rs;

	rs = m_db->executeQuery(L"select * from Statistics");
	if (!rs)
		return false;

	while (rs->next())
	{
		outStats.insert(std::make_pair(
			rs->getString(L"id"),
			rs->getInt32(L"value")
		));
	}

	return true;
}

bool LocalStatistics::set(const std::wstring& statId, int32_t value)
{
	if (m_db->executeUpdate(L"update Statistics set value=" + toString(value) + L" where id='" + statId + L"'") > 0)
		return true;
	else
		return false;
}

	}
}
