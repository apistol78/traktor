/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Io/IStream.h"
#include "Core/Serialization/ISerializable.h"
#include "Database/Database.h"
#include "Database/DatabaseClassFactory.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"

namespace traktor
{
	namespace db
	{
		namespace
		{

Ref< Instance > db_Database_getInstanceByGuid(Database* self, const Guid& instanceGuid)
{
	return self->getInstance(instanceGuid);
}

Ref< Instance > db_Database_getInstanceByPath(Database* self, const std::wstring& instancePath)
{
	return self->getInstance(instancePath);
}

Ref< Instance > db_Database_createInstance(Database* self, const std::wstring& instancePath)
{
	return self->createInstance(instancePath);
}

Ref< ISerializable > db_Database_getObjectReadOnly(Database* self, const Guid& id)
{
	return self->getObjectReadOnly(id);
}

RefArray< Instance > db_Database_findInstancesByType(Database* self, const TypeInfo& instanceType)
{
	RefArray< Instance > instances;
	recursiveFindChildInstances(self->getRootGroup(), FindInstanceByType(instanceType), instances);
	return instances;
}

Ref< Instance > db_Group_getInstanceByName(Group* self, const std::wstring& instanceName)
{
	return self->getInstance(instanceName);
}

Ref< Instance > db_Group_createInstance(Group* self, const std::wstring& instancePath)
{
	return self->createInstance(instancePath);
}

RefArray< Group > db_Group_getChildGroups(Group* self)
{
	RefArray< Group > childGroups;
	self->getChildGroups(childGroups);
	return childGroups;
}

RefArray< Instance > db_Group_getChildInstances(Group* self)
{
	RefArray< Instance > childInstances;
	self->getChildInstances(childInstances);
	return childInstances;
}

Any db_Instance_getPrimaryType(Instance* self)
{
	const TypeInfo* primaryType = self->getPrimaryType();
	return primaryType ? CastAny< TypeInfo, false >::set(*primaryType) : Any();
}

Ref< ISerializable > db_Instance_getObject(Instance* self)
{
	return self->getObject();
}

std::vector< std::wstring > db_Instance_getDataNames(Instance* self)
{
	std::vector< std::wstring > dataNames;
	self->getDataNames(dataNames);
	return dataNames;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DatabaseClassFactory", 0, DatabaseClassFactory, IRuntimeClassFactory)

void DatabaseClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< Database > > classDatabase = new AutoRuntimeClass< Database >();
	classDatabase->addMethod("close", &Database::close);
	classDatabase->addMethod("getRootGroup", &Database::getRootGroup);
	classDatabase->addMethod("getGroup", &Database::getGroup);
	classDatabase->addMethod("createGroup", &Database::createGroup);
	classDatabase->addMethod("getInstanceByGuid", &db_Database_getInstanceByGuid);
	classDatabase->addMethod("getInstanceByPath", &db_Database_getInstanceByPath);
	classDatabase->addMethod("createInstance", &db_Database_createInstance);
	classDatabase->addMethod("getObjectReadOnly", &db_Database_getObjectReadOnly);
	classDatabase->addMethod("findInstancesByType", &db_Database_findInstancesByType);
	registrar->registerClass(classDatabase);

	Ref< AutoRuntimeClass< Group > > classGroup = new AutoRuntimeClass< Group >();
	classGroup->addMethod("getName", &Group::getName);
	classGroup->addMethod("getPath", &Group::getPath);
	classGroup->addMethod("rename", &Group::rename);
	classGroup->addMethod("remove", &Group::remove);
	classGroup->addMethod("getGroup", &Group::getGroup);
	classGroup->addMethod("createGroup", &Group::createGroup);
	classGroup->addMethod("getInstanceByName", &db_Group_getInstanceByName);
	classGroup->addMethod("createInstance", &db_Group_createInstance);
	classGroup->addMethod("getParent", &Group::getParent);
	classGroup->addMethod("getChildGroups", &db_Group_getChildGroups);
	classGroup->addMethod("getChildInstances", &db_Group_getChildInstances);
	registrar->registerClass(classGroup);

	Ref< AutoRuntimeClass< Instance > > classInstance = new AutoRuntimeClass< Instance >();
	classInstance->addMethod("getParent", &Instance::getParent);
	classInstance->addMethod("getName", &Instance::getName);
	classInstance->addMethod("getPath", &Instance::getPath);
	classInstance->addMethod("getGuid", &Instance::getGuid);
	classInstance->addMethod("getPrimaryTypeName", &Instance::getPrimaryTypeName);
	classInstance->addMethod("getPrimaryType", &db_Instance_getPrimaryType);
	classInstance->addMethod("getObject", &db_Instance_getObject);
	classInstance->addMethod("getDataNames", &db_Instance_getDataNames);
	classInstance->addMethod("readData", &Instance::readData);
	classInstance->addMethod("checkout", &Instance::checkout);
	classInstance->addMethod("commit", &Instance::commit);
	classInstance->addMethod("revert", &Instance::revert);
	classInstance->addMethod("remove", &Instance::remove);
	classInstance->addMethod("setName", &Instance::setName);
	classInstance->addMethod("setGuid", &Instance::setGuid);
	classInstance->addMethod("setObject", &Instance::setObject);
	classInstance->addMethod("removeAllData", &Instance::removeAllData);
	classInstance->addMethod("writeData", &Instance::writeData);
	registrar->registerClass(classInstance);
}

	}
}
