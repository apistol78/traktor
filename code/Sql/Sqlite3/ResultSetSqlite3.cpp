/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <sqlite3.h>
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Sql/Sqlite3/ResultSetSqlite3.h"

namespace traktor
{
	namespace sql
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sql.ResultSetSqlite3", ResultSetSqlite3, IResultSet)

bool ResultSetSqlite3::next()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!m_stmt)
		return false;

	int err = sqlite3_step((sqlite3_stmt*)m_stmt);
	
	if (err == SQLITE_ROW)
		return true;
	else if (err == SQLITE_DONE || err == SQLITE_ERROR)
	{
		sqlite3_finalize((sqlite3_stmt*)m_stmt);
		m_stmt = 0;
	}

	return false;
}

int32_t ResultSetSqlite3::getColumnCount() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	return sqlite3_column_count((sqlite3_stmt*)m_stmt);
}

std::wstring ResultSetSqlite3::getColumnName(int32_t columnIndex) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	const char* name = sqlite3_column_origin_name((sqlite3_stmt*)m_stmt, columnIndex);
	return name ? mbstows(name) : L"";
}

ColumnType ResultSetSqlite3::getColumnType(int32_t columnIndex) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	int type = sqlite3_column_type((sqlite3_stmt*)m_stmt, columnIndex);
	switch (type)
	{
	case SQLITE_INTEGER:
		return CtInt64;
	case SQLITE_FLOAT:
		return CtDouble;
	case SQLITE_TEXT:
		return CtString;
	}
	return CtVoid;
}

int32_t ResultSetSqlite3::getInt32(int32_t columnIndex) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	return sqlite3_column_int((sqlite3_stmt*)m_stmt, columnIndex);
}

int64_t ResultSetSqlite3::getInt64(int32_t columnIndex) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	return sqlite3_column_int64((sqlite3_stmt*)m_stmt, columnIndex);
}

float ResultSetSqlite3::getFloat(int32_t columnIndex) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	return (float)sqlite3_column_double((sqlite3_stmt*)m_stmt, columnIndex);
}

double ResultSetSqlite3::getDouble(int32_t columnIndex) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	return sqlite3_column_double((sqlite3_stmt*)m_stmt, columnIndex);
}

std::wstring ResultSetSqlite3::getString(int32_t columnIndex) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	const char* text = (const char*)sqlite3_column_text((sqlite3_stmt*)m_stmt, columnIndex);
	return text ? mbstows(text) : L"";
}

ResultSetSqlite3::ResultSetSqlite3(Semaphore& lock, void* stmt)
:	m_lock(lock)
,	m_stmt(stmt)
{
}

ResultSetSqlite3::~ResultSetSqlite3()
{
	if (m_stmt)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		sqlite3_finalize((sqlite3_stmt*)m_stmt);
	}
}

	}
}
