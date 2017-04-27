/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Sql/Sqlite3/ConnectionSqlite3.h"
#include "Sql/Sqlite3/Sqlite3ClassFactory.h"

namespace traktor
{
	namespace sql
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sql.Sqlite3ClassFactory", 0, Sqlite3ClassFactory, IRuntimeClassFactory)

void Sqlite3ClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< sql::ConnectionSqlite3 > > classConnectionSqlite3 = new AutoRuntimeClass< sql::ConnectionSqlite3 >();
	classConnectionSqlite3->addConstructor();
	registrar->registerClass(classConnectionSqlite3);
}

	}
}
