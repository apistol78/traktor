#ifndef traktor_sql_Sqlite3ClassFactory_H
#define traktor_sql_Sqlite3ClassFactory_H

#include "Core/Class/IRuntimeClassFactory.h"

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

class T_DLLCLASS Sqlite3ClassFactory : public IRuntimeClassFactory
{
	T_RTTI_CLASS;

public:
	virtual void createClasses(IRuntimeClassRegistrar* registrar) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_sql_Sqlite3ClassFactory_H
