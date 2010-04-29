#include "Core/Log/Log.h"
#include "Core/System/OS.h"
#include "Online/Local/SessionLocal.h"
#include "Online/Local/SessionManagerLocal.h"
#include "Online/Local/UserLocal.h"
#include "Sql/IResultSet.h"
#include "Sql/Sqlite3/ConnectionSqlite3.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.online.SessionManagerLocal", 0, SessionManagerLocal, ISessionManager)

SessionManagerLocal::SessionManagerLocal()
{
}

bool SessionManagerLocal::create()
{
	Ref< sql::IResultSet > rs;

	// Get current user; use logged in OS user.
	std::wstring user = OS::getInstance().getCurrentUser();
	if (user.empty())
	{
		log::error << L"Unable to create local session manager; no user" << Endl;
		return false;
	}

	// Open sessions database.
	std::wstring sessionDb = OS::getInstance().getUserApplicationDataPath() + L"/Doctor Entertainment AB/Sessions.db";
	log::debug << L"Using session database \"" << sessionDb << L"\"" << Endl;

	m_db = new sql::ConnectionSqlite3();
	if (!m_db->connect(L"fileName=" + sessionDb))
	{
		// \hack Try to create database in current working directory.
		if (!m_db->connect(L"fileName=Sessions.db"))
		{
			log::error << L"Unable to create local session manager; unable to connect to database" << Endl;
			return false;
		}
	}

	if (!m_db->tableExists(L"Users"))
	{
		// Create users table.
		if (m_db->executeUpdate(L"create table Users (id integer primary key, name varchar(250))") < 0)
		{
			log::error << L"Unable to create local session manager; unable to create table Users" << Endl;
			return false;
		}
	}

	if (!m_db->tableExists(L"Friends"))
	{
		// Create friends table.
		if (m_db->executeUpdate(L"create table Friends (userId integer, friendId integer)") < 0)
		{
			log::error << L"Unable to create local session manager; unable to create table Friends" << Endl;
			return false;
		}
	}

	if (!m_db->tableExists(L"Achievements"))
	{
		// Create achievements table.
		if (m_db->executeUpdate(L"create table Achievements (id integer primary key, name varchar(250))") < 0)
		{
			log::error << L"Unable to create local session manager; unable to create table Achievements" << Endl;
			return false;
		}
	}

	if (!m_db->tableExists(L"AwardedAchievements"))
	{
		// Create awarded achievements table.
		if (m_db->executeUpdate(L"create table AwardedAchievements (userId integer, achievementId integer)") < 0)
		{
			log::error << L"Unable to create local session manager; unable to create table AwardedAchievements" << Endl;
			return false;
		}
	}

	if (!m_db->tableExists(L"SaveGames"))
	{
		// Create save games table.
		if (m_db->executeUpdate(L"create table SaveGames (id integer primary key, name varchar(250), attachment varchar(4096))") < 0)
		{
			log::error << L"Unable to create local session manager; unable to create table SaveGames" << Endl;
			return false;
		}
	}

	// Get user identity in database.
	int32_t userId;

	rs = m_db->executeQuery(L"select id from Users where name='" + user + L"'");
	if (rs && rs->next())
	{
		userId = rs->getInt32(0);
	}
	else
	{
		// User not in database; create new user entry.
		if (m_db->executeUpdate(L"insert into Users (name) values ('" + user + L"')") < 0)
		{
			log::error << L"Unable to create local session manager; unable to create table Users" << Endl;
			return false;
		}

		userId = m_db->lastInsertId();
	}

	m_currentUser = new UserLocal(m_db, userId, user);

	return true;
}

void SessionManagerLocal::destroy()
{
	m_currentUser = 0;

	if (m_db)
	{
		m_db->disconnect();
		m_db = 0;
	}
}

bool SessionManagerLocal::getAvailableUsers(RefArray< IUser >& outUsers)
{
	Ref< sql::IResultSet > rs = m_db->executeQuery(L"select * from Users");
	if (!rs)
		return false;

	while (rs->next())
	{
		outUsers.push_back(new UserLocal(
			m_db,
			rs->getInt32(L"id"),
			rs->getString(L"name")
		));
	}

	return true;
}

Ref< IUser > SessionManagerLocal::getCurrentUser()
{
	return m_currentUser;
}

Ref< ISession > SessionManagerLocal::createSession(IUser* user)
{
	T_ASSERT (user == m_currentUser);
	return new SessionLocal(m_db, checked_type_cast< UserLocal*, false >(user));
}

bool SessionManagerLocal::update()
{
	return true;
}

	}
}
