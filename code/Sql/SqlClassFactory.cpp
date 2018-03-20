/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Sql/IConnection.h"
#include "Sql/IResultSet.h"
#include "Sql/SqlClassFactory.h"

namespace traktor
{
	namespace sql
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sql.SqlClassFactory", 0, SqlClassFactory, IRuntimeClassFactory)

void SqlClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< sql::IResultSet > > classIResultSet = new AutoRuntimeClass< sql::IResultSet >();
	classIResultSet->addMethod("next", &sql::IResultSet::next);
	classIResultSet->addMethod("getColumnCount", &sql::IResultSet::getColumnCount);
	classIResultSet->addMethod("getColumnName", &sql::IResultSet::getColumnName);
	classIResultSet->addMethod< int32_t, int32_t >("getInt32", &sql::IResultSet::getInt32);
	classIResultSet->addMethod< int64_t, int32_t >("getInt64", &sql::IResultSet::getInt64);
	classIResultSet->addMethod< float, int32_t >("getFloat", &sql::IResultSet::getFloat);
	classIResultSet->addMethod< double, int32_t >("getDouble", &sql::IResultSet::getDouble);
	classIResultSet->addMethod< std::wstring, int32_t >("getString", &sql::IResultSet::getString);
	classIResultSet->addMethod("findColumn", &sql::IResultSet::findColumn);
	classIResultSet->addMethod< int32_t, const std::wstring& >("getInt32ByName", &sql::IResultSet::getInt32);
	classIResultSet->addMethod< int64_t, const std::wstring& >("getInt64ByName", &sql::IResultSet::getInt64);
	classIResultSet->addMethod< float, const std::wstring& >("getFloatByName", &sql::IResultSet::getFloat);
	classIResultSet->addMethod< double, const std::wstring& >("getDoubleByName", &sql::IResultSet::getDouble);
	classIResultSet->addMethod< std::wstring, const std::wstring& >("getStringByName", &sql::IResultSet::getString);
	registrar->registerClass(classIResultSet);

	Ref< AutoRuntimeClass< sql::IConnection > > classIConnection = new AutoRuntimeClass< sql::IConnection >();
	classIConnection->addMethod("connect", &sql::IConnection::connect);
	classIConnection->addMethod("disconnect", &sql::IConnection::disconnect);
	classIConnection->addMethod("executeQuery", &sql::IConnection::executeQuery);
	classIConnection->addMethod("executeUpdate", &sql::IConnection::executeUpdate);
	classIConnection->addMethod("lastInsertId", &sql::IConnection::lastInsertId);
	classIConnection->addMethod("tableExists", &sql::IConnection::tableExists);
	registrar->registerClass(classIConnection);
}

	}
}
