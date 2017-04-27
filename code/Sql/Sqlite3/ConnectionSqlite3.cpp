/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <map>
#include <sqlite3.h>
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Core/Thread/Acquire.h"
#include "Sql/Sqlite3/ConnectionSqlite3.h"
#include "Sql/Sqlite3/ResultSetSqlite3.h"

namespace traktor
{
	namespace sql
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sql.ConnectionSqlite3", 0, ConnectionSqlite3, IConnection)

ConnectionSqlite3::ConnectionSqlite3()
:	m_db(0)
{
}

bool ConnectionSqlite3::connect(const std::wstring& connectionString)
{
	std::vector< std::wstring > pairs;
	std::map< std::wstring, std::wstring > cs;

	if (Split< std::wstring >::any(connectionString, L";", pairs) == 0)
		return false;

	for (std::vector< std::wstring >::const_iterator i = pairs.begin(); i != pairs.end(); ++i)
	{
		size_t p = i->find(L'=');
		if (p == 0 || p == i->npos)
			return false;

		cs[trim(i->substr(0, p))] = i->substr(p + 1);
	}

	Path fileName = FileSystem::getInstance().getAbsolutePath(cs[L"fileName"]);
#if !defined(_PS3)
	FileSystem::getInstance().makeAllDirectories(fileName.getPathOnly());
#endif

#if defined(TARGET_OS_MAC) || defined(__LINUX__) || defined(__ANDROID__)
	std::wstring dbName = fileName.getPathNameNoVolume();
#else
	std::wstring dbName = fileName.getPathName();
#endif

	T_DEBUG(L"Using SQLite db \"" << dbName << L"\"");

	sqlite3* db = 0;

	int err = sqlite3_open(
		wstombs(dbName).c_str(),
		&db
	);
	if (err != SQLITE_OK)
	{
		log::error << L"In connect, sqlite3_open failed (" << dbName << L"):" << Endl;
		log::error << mbstows(sqlite3_errmsg((sqlite3*)m_db)) << Endl;
		return false;
	}

	m_db = (void*)db;
	return true;
}

void ConnectionSqlite3::disconnect()
{
	if (m_db)
	{
		sqlite3_close((sqlite3*)m_db);
		m_db = 0;
	}
}

Ref< IResultSet > ConnectionSqlite3::executeQuery(const std::wstring& query)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	sqlite3_stmt* stmt = 0;

	int err = sqlite3_prepare_v2(
		(sqlite3*)m_db,
		wstombs(query).c_str(),
		query.length(),
		&stmt,
		0
	);
	if (err != SQLITE_OK)
	{
		log::error << L"In executeQuery, sqlite3_prepare_v2 failed:" << Endl;
		log::error << mbstows(sqlite3_errmsg((sqlite3*)m_db)) << Endl;
		return 0;
	}

	return new ResultSetSqlite3(m_lock, (void*)stmt);
}

int32_t ConnectionSqlite3::executeUpdate(const std::wstring& update)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	sqlite3_stmt* stmt = 0;
	int err;

	err = sqlite3_prepare_v2(
		(sqlite3*)m_db,
		wstombs(update).c_str(),
		update.length(),
		&stmt,
		0
	);
	if (err != SQLITE_OK)
	{
		log::error << L"In executeUpdate, sqlite3_prepare_v2 failed:" << Endl;
		log::error << mbstows(sqlite3_errmsg((sqlite3*)m_db)) << Endl;
		return 0;
	}

	err = sqlite3_step((sqlite3_stmt*)stmt);
	sqlite3_finalize((sqlite3_stmt*)stmt);

	if (err != SQLITE_DONE)
		return -1;

	return sqlite3_changes((sqlite3*)m_db);
}

int32_t ConnectionSqlite3::lastInsertId()
{
	Ref< sql::IResultSet > rs = executeQuery(L"select last_insert_rowid() as id");
	return (rs && rs->next()) ? rs->getInt32(0) : - 1;
}

bool ConnectionSqlite3::tableExists(const std::wstring& tableName)
{
	Ref< sql::IResultSet > rs = executeQuery(L"select count(*) from sqlite_master where name='" + tableName + L"'");
	return (rs && rs->next()) ? (rs->getInt32(0) > 0) : false;
}

	}
}
