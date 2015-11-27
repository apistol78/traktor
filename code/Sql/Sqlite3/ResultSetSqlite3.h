#ifndef traktor_sql_ResultSetSqlite3_H
#define traktor_sql_ResultSetSqlite3_H

#include "Sql/IResultSet.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SQL_SQLITE3_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Semaphore;

	namespace sql
	{

/*! \brief Sqlite3 query/update result set.
 * \ingroup SQL
 */
class T_DLLCLASS ResultSetSqlite3 : public IResultSet
{
	T_RTTI_CLASS;

public:
	virtual bool next() T_OVERRIDE T_FINAL;

	virtual int32_t getColumnCount() const T_OVERRIDE T_FINAL;

	virtual std::wstring getColumnName(int32_t columnIndex) const T_OVERRIDE T_FINAL;

	virtual ColumnType getColumnType(int32_t columnIndex) const T_OVERRIDE T_FINAL;

	virtual int32_t getInt32(int32_t columnIndex) const T_OVERRIDE T_FINAL;

	virtual int64_t getInt64(int32_t columnIndex) const T_OVERRIDE T_FINAL;

	virtual float getFloat(int32_t columnIndex) const T_OVERRIDE T_FINAL;

	virtual double getDouble(int32_t columnIndex) const T_OVERRIDE T_FINAL;

	virtual std::wstring getString(int32_t columnIndex) const T_OVERRIDE T_FINAL;

private:
	friend class ConnectionSqlite3;

	Semaphore& m_lock;
	void* m_stmt;

	ResultSetSqlite3(Semaphore& lock, void* stmt);

	virtual ~ResultSetSqlite3();
};

	}
}

#endif	// traktor_sql_ResultSetSqlite3_H
