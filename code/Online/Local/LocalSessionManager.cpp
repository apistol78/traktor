#include "Core/System/OS.h"
#include "Online/Local/LocalAchievements.h"
#include "Online/Local/LocalGameConfiguration.h"
#include "Online/Local/LocalLeaderboards.h"
#include "Online/Local/LocalMatchMaking.h"
#include "Online/Local/LocalSaveData.h"
#include "Online/Local/LocalStatistics.h"
#include "Online/Local/LocalSessionManager.h"
#include "Online/Local/LocalUser.h"
#include "Sql/IResultSet.h"
#include "Sql/Sqlite3/ConnectionSqlite3.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.online.LocalSessionManager", 0, LocalSessionManager, ISessionManagerProvider)

bool LocalSessionManager::create(const IGameConfiguration* configuration)
{
	const LocalGameConfiguration* gc = dynamic_type_cast< const LocalGameConfiguration* >(configuration);
	if (!gc)
		return false;

#if TARGET_OS_IPHONE
	std::wstring dbPath = OS::getInstance().getWritableFolderPath() + L"/" + std::wstring(gc->m_dbName) + L".db";
#else
	std::wstring dbPath = OS::getInstance().getWritableFolderPath() + L"/Doctor Entertainment AB/" + std::wstring(gc->m_dbName) + L".db";
#endif

	m_db = new sql::ConnectionSqlite3();
	if (!m_db->connect(L"fileName=" + dbPath))
	{
		if (!m_db->connect(L"fileName=" + std::wstring(gc->m_dbName) + L".db"))
			return false;
	}

	if (!m_db->tableExists(L"Achievements"))
	{
		if (m_db->executeUpdate(L"create table Achievements (id varchar(64) primary key, reward integer)") < 0)
			return false;

		for (std::list< std::wstring >::const_iterator i = gc->m_achievementIds.begin(); i != gc->m_achievementIds.end(); ++i)
		{
			if (m_db->executeUpdate(L"insert into Achievements (id, reward) values ('" + *i + L"', 0)") < 0)
				return false;
		}
	}
	else
	{
		for (std::list< std::wstring >::const_iterator i = gc->m_achievementIds.begin(); i != gc->m_achievementIds.end(); ++i)
		{
			Ref< sql::IResultSet > rs = m_db->executeQuery(L"select count(*) from Achievements where id='" + *i + L"'");
			if (!rs || !rs->next() || rs->getInt32(0) <= 0)
			{
				if (m_db->executeUpdate(L"insert into Achievements (id, reward) values ('" + *i + L"', 0)") < 0)
					return false;
			}
		}
	}

	if (!m_db->tableExists(L"Leaderboards"))
	{
		if (m_db->executeUpdate(L"create table Leaderboards (id integer primary key, name varchar(64), score integer)") < 0)
			return false;

		for (std::list< std::wstring >::const_iterator i = gc->m_leaderboardIds.begin(); i != gc->m_leaderboardIds.end(); ++i)
		{
			if (m_db->executeUpdate(L"insert into Leaderboards (name, score) values ('" + *i + L"', 0)") < 0)
				return false;
		}
	}
	else
	{
		for (std::list< std::wstring >::const_iterator i = gc->m_leaderboardIds.begin(); i != gc->m_leaderboardIds.end(); ++i)
		{
			Ref< sql::IResultSet > rs = m_db->executeQuery(L"select count(*) from Leaderboards where name='" + *i + L"'");
			if (!rs || !rs->next() || rs->getInt32(0) <= 0)
			{
				if (m_db->executeUpdate(L"insert into Leaderboards (name, score) values ('" + *i + L"', 0)") < 0)
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
		if (m_db->executeUpdate(L"create table Statistics (id varchar(64) primary key, value integer)") < 0)
			return false;

		for (std::list< std::wstring >::const_iterator i = gc->m_statsIds.begin(); i != gc->m_statsIds.end(); ++i)
		{
			if (m_db->executeUpdate(L"insert into Statistics (id, value) values ('" + *i + L"', 0)") < 0)
				return false;
		}
	}
	else
	{
		for (std::list< std::wstring >::const_iterator i = gc->m_statsIds.begin(); i != gc->m_statsIds.end(); ++i)
		{
			Ref< sql::IResultSet > rs = m_db->executeQuery(L"select count(*) from Statistics where id='" + *i + L"'");
			if (!rs || !rs->next() || rs->getInt32(0) <= 0)
			{
				if (m_db->executeUpdate(L"insert into Statistics (id, value) values ('" + *i + L"', 0)") < 0)
					return false;
			}
		}
	}

	m_achievements = new LocalAchievements(m_db);
	m_leaderboards = new LocalLeaderboards(m_db);
	m_matchMaking = new LocalMatchMaking();
	m_saveData = new LocalSaveData(m_db);
	m_statistics = new LocalStatistics(m_db);
	m_user = new LocalUser();

	return true;
}

void LocalSessionManager::destroy()
{
	m_user = 0;
	m_statistics = 0;
	m_saveData = 0;
	m_matchMaking = 0;
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

bool LocalSessionManager::haveDLC(const std::wstring& id) const
{
	return false;
}

bool LocalSessionManager::buyDLC(const std::wstring& id) const
{
	return false;
}

bool LocalSessionManager::navigateUrl(const net::Url& url) const
{
	return false;
}

uint64_t LocalSessionManager::getCurrentUserHandle() const
{
	return 0;
}

bool LocalSessionManager::getFriends(std::vector< uint64_t >& outFriends, bool onlineOnly) const
{
	return true;
}

bool LocalSessionManager::findFriend(const std::wstring& name, uint64_t& outFriendUserHandle) const
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
	return m_matchMaking;
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
	return m_user;
}

	}
}
