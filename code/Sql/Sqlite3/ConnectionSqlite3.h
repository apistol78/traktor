/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sql_ConnectionSqlite3_H
#define traktor_sql_ConnectionSqlite3_H

#include "Core/Thread/Semaphore.h"
#include "Sql/IConnection.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SQL_SQLITE3_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sql
	{

/*! \brief Sqlite3 database connection.
 * \ingroup SQL
 *
 * Connection string definition:
 * {key=value};*
 *
 * fileName	Database filename.
 */
class T_DLLCLASS ConnectionSqlite3 : public IConnection
{
	T_RTTI_CLASS;

public:
	ConnectionSqlite3();

	virtual bool connect(const std::wstring& connectionString) T_OVERRIDE T_FINAL;

	virtual void disconnect() T_OVERRIDE T_FINAL;

	virtual Ref< IResultSet > executeQuery(const std::wstring& query) T_OVERRIDE T_FINAL;

	virtual int32_t executeUpdate(const std::wstring& update) T_OVERRIDE T_FINAL;

	virtual int32_t lastInsertId() T_OVERRIDE T_FINAL;

	virtual bool tableExists(const std::wstring& tableName) T_OVERRIDE T_FINAL;

private:
	Semaphore m_lock;
	void* m_db;
};

	}
}

#endif	// traktor_sql_ConnectionSqlite3_H
