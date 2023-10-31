/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Sql/IConnection.h"
#include "Sql/IResultSet.h"
#include "Sql/SqlClassFactory.h"

namespace traktor::sql
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sql.SqlClassFactory", 0, SqlClassFactory, IRuntimeClassFactory)

void SqlClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< IResultSet > > classIResultSet = new AutoRuntimeClass< IResultSet >();
	classIResultSet->addProperty("columnCount", &IResultSet::getColumnCount);
	classIResultSet->addMethod("next", &IResultSet::next);
	classIResultSet->addMethod("getColumnName", &IResultSet::getColumnName);
	classIResultSet->addMethod< int32_t, int32_t >("getInt32", &IResultSet::getInt32);
	classIResultSet->addMethod< int64_t, int32_t >("getInt64", &IResultSet::getInt64);
	classIResultSet->addMethod< float, int32_t >("getFloat", &IResultSet::getFloat);
	classIResultSet->addMethod< double, int32_t >("getDouble", &IResultSet::getDouble);
	classIResultSet->addMethod< std::wstring, int32_t >("getString", &IResultSet::getString);
	classIResultSet->addMethod("findColumn", &IResultSet::findColumn);
	classIResultSet->addMethod< int32_t, const std::wstring& >("getInt32ByName", &IResultSet::getInt32);
	classIResultSet->addMethod< int64_t, const std::wstring& >("getInt64ByName", &IResultSet::getInt64);
	classIResultSet->addMethod< float, const std::wstring& >("getFloatByName", &IResultSet::getFloat);
	classIResultSet->addMethod< double, const std::wstring& >("getDoubleByName", &IResultSet::getDouble);
	classIResultSet->addMethod< std::wstring, const std::wstring& >("getStringByName", &IResultSet::getString);
	registrar->registerClass(classIResultSet);

	Ref< AutoRuntimeClass< IConnection > > classIConnection = new AutoRuntimeClass< IConnection >();
	classIConnection->addProperty("lastInsertId", &IConnection::lastInsertId);
	classIConnection->addMethod("connect", &IConnection::connect);
	classIConnection->addMethod("disconnect", &IConnection::disconnect);
	classIConnection->addMethod("executeQuery", &IConnection::executeQuery);
	classIConnection->addMethod("executeUpdate", &IConnection::executeUpdate);
	classIConnection->addMethod("tableExists", &IConnection::tableExists);
	registrar->registerClass(classIConnection);
}

}
