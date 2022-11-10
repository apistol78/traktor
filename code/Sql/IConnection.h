/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SQL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sql
	{

class IResultSet;

/*! SQL database connection.
 * \ingroup SQL
 */
class T_DLLCLASS IConnection : public Object
{
	T_RTTI_CLASS;

public:
	/*! Connect to database.
	 *
	 * \param connectionString Database connection string.
	 * \return True if successful.
	 */
	virtual bool connect(const std::wstring& connectionString) = 0;

	/*! Disconnect from database. */
	virtual void disconnect() = 0;

	/*! Execute query statement.
	 *
	 * \param query Query statement.
	 * \return Result set; null if failed.
	 */
	virtual Ref< IResultSet > executeQuery(const std::wstring& query) = 0;

	/*! Execute update statement.
	 *
	 * \param update Update statement.
	 * \return Number of rows affected, -1 if failed.
	 */
	virtual int32_t executeUpdate(const std::wstring& update) = 0;

	/*! Get last auto-generated id used with insert.
	 *
	 * \return Last insert id.
	 */
	virtual int32_t lastInsertId() = 0;

	/*! Have table query.
	 *
	 * Different providers have different
	 * ways of storing table information thus
	 * we need a way to abstract it.
	 *
	 * \param tableName Name of table.
	 * \return True if table exists.
	 */
	virtual bool tableExists(const std::wstring& tableName) = 0;
};

	}
}

