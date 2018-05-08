/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sql_IConnection_H
#define traktor_sql_IConnection_H

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

/*! \brief SQL database connection.
 * \ingroup SQL
 */
class T_DLLCLASS IConnection : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Connect to database.
	 *
	 * \param connectionString Database connection string.
	 * \return True if successful.
	 */
	virtual bool connect(const std::wstring& connectionString) = 0;

	/*! \brief Disconnect from database. */
	virtual void disconnect() = 0;

	/*! \brief Execute query statement.
	 *
	 * \param query Query statement.
	 * \return Result set; null if failed.
	 */
	virtual Ref< IResultSet > executeQuery(const std::wstring& query) = 0;

	/*! \brief Execute update statement.
	 *
	 * \param update Update statement.
	 * \return Number of rows affected, -1 if failed.
	 */
	virtual int32_t executeUpdate(const std::wstring& update) = 0;

	/*! \brief Get last auto-generated id used with insert.
	 *
	 * \return Last insert id.
	 */
	virtual int32_t lastInsertId() = 0;

	/*! \brief Have table query.
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

#endif	// traktor_sql_IConnection_H
