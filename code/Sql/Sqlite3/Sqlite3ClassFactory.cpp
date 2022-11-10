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
