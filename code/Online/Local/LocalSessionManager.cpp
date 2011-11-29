#include "Core/System/OS.h"
#include "Online/Local/LocalAchievements.h"
#include "Online/Local/LocalLeaderboards.h"
#include "Online/Local/LocalSaveData.h"
#include "Online/Local/LocalStatistics.h"
#include "Online/Local/LocalSessionManager.h"
#include "Sql/IResultSet.h"
#include "Sql/Sqlite3/ConnectionSqlite3.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LocalSessionManager", LocalSessionManager, ISessionManagerProvider)

bool LocalSessionManager::create(const LocalCreateDesc& desc)
{
#if TARGET_OS_IPHONE
	std::wstring dbPath = OS::getInstance().getWritableFolderPath() + L"/" + std::wstring(desc.dbName) + L".db";
#else
	std::wstring dbPath = OS::getInstance().getWritableFolderPath() + L"/Doctor Entertainment AB/" + std::wstring(desc.dbName) + L".db";
#endif

	m_db = new sql::ConnectionSqlite3();
	if (!m_db->connect(L"fileName=" + dbPath))
	{
		if (!m_db->connect(L"fileName=" + std::wstring(desc.dbName) + L".db"))
			return false;
	}

	if (!m_db->tableExists(L"Achievements"))
	{
		if (m_db->executeUpdate(L"create table Achievements (id varchar(64) primary key, reward integer)") < 0)
			return false;

		for (const wchar_t** achievementId = desc.achievementIds; *achievementId; ++achievementId)
		{
			if (m_db->executeUpdate(L"insert into Achievements (id, reward) values ('" + std::wstring(*achievementId) + L"', 0)") < 0)
				return false;
		}
	}
	else
	{
		for (const wchar_t** achievementId = desc.achievementIds; *achievementId; ++achievementId)
		{
			Ref< sql::IResultSet > rs = m_db->executeQuery(L"select count(*) from Achievements where id='" + std::wstring(*achievementId) + L"'");
			if (!rs || !rs->next() || rs->getInt32(0) <= 0)
			{
				if (m_db->executeUpdate(L"insert into Achievements (id, reward) values ('" + std::wstring(*achievementId) + L"', 0)") < 0)
					return false;
			}
		}
	}

	if (!m_db->tableExists(L"Leaderboards"))
	{
		if (m_db->executeUpdate(L"create table Leaderboards (id integer primary key, name varchar(64), score integer)") < 0)
			return false;

		for (const wchar_t** leaderboardId = desc.leaderboardIds; *leaderboardId; ++leaderboardId)
		{
			if (m_db->executeUpdate(L"insert into Leaderboards (name, score) values ('" + std::wstring(*leaderboardId) + L"', 0)") < 0)
				return false;
		}
	}
	else
	{
		for (const wchar_t** leaderboardId = desc.leaderboardIds; *leaderboardId; ++leaderboardId)
		{
			Ref< sql::IResultSet > rs = m_db->executeQuery(L"select count(*) from Leaderboards where name='" + std::wstring(*leaderboardId) + L"'");
			if (!rs || !rs->next() || rs->getInt32(0) <= 0)
			{
				if (m_db->executeUpdate(L"insert into Leaderboards (name, score) values ('" + std::wstring(*leaderboardId) + L"', 0)") < 0)
					return false;
			}
		}
	}

	if (!m_db->tableExists(L"SaveData"))
	{
		if (m_db->executeUpdate(L"create table SaveData (id varchar(64) primary key, attachment varchar(4096))") < 0)
			return false;
	}

	if (!m_db->tableExists(L"Statistics"))
	{
		if (m_db->executeUpdate(L"create table Statistics (id varchar(64) primary key, value float)") < 0)
			return false;

		for (const wchar_t** statId = desc.statIds; *statId; ++statId)
		{
			if (m_db->executeUpdate(L"insert into Statistics (id, value) values ('" + std::wstring(*statId) + L"', 0)") < 0)
				return false;
		}
	}
	else
	{
		for (const wchar_t** statId = desc.statIds; *statId; ++statId)
		{
			Ref< sql::IResultSet > rs = m_db->executeQuery(L"select count(*) from Statistics where id='" + std::wstring(*statId) + L"'");
			if (!rs || !rs->next() || rs->getInt32(0) <= 0)
			{
				if (m_db->executeUpdate(L"insert into Statistics (id, value) values ('" + std::wstring(*statId) + L"', 0)") < 0)
					return false;
			}
		}
	}

	m_achievements = new LocalAchievements(m_db);
	m_leaderboards = new LocalLeaderboards(m_db);
	m_saveData = new LocalSaveData(m_db);
	m_statistics = new LocalStatistics(m_db);

	return true;
}

void LocalSessionManager::destroy()
{
	m_statistics = 0;
	m_saveData = 0;
	m_leaderboards = 0;
	m_achievements = 0;

	if (m_db)
	{
		m_db->disconnect();
		m_db = 0;
	}
}

bool LocalSessionManager::update()
{
	return true;
}

std::wstring LocalSessionManager::getLanguageCode() const
{
	return L"";
}

bool LocalSessionManager::isConnected() const
{
	return true;
}

bool LocalSessionManager::requireUserAttention() const
{
	return false;
}

bool LocalSessionManager::haveP2PData() const
{
	return false;
}

uint32_t LocalSessionManager::receiveP2PData(void* data, uint32_t size, uint64_t& outFromUserHandle) const
{
	return 0;
}

IAchievementsProvider* LocalSessionManager::getAchievements() const
{
	return m_achievements;
}

ILeaderboardsProvider* LocalSessionManager::getLeaderboards() const
{
	return m_leaderboards;
}

IMatchMakingProvider* LocalSessionManager::getMatchMaking() const
{
	return 0;
}

ISaveDataProvider* LocalSessionManager::getSaveData() const
{
	return m_saveData;
}

IStatisticsProvider* LocalSessionManager::getStatistics() const
{
	return m_statistics;
}

IUserProvider* LocalSessionManager::getUser() const
{
	return 0;
}

	}
}
