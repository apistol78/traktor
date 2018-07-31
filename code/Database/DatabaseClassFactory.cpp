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

Ref< Instance > Database_getInstanceByGuid(Database* self, const Guid& instanceGuid)
{
	return self->getInstance(instanceGuid);
}

Ref< Instance > Database_getInstanceByPath(Database* self, const std::wstring& instancePath)
{
	return self->getInstance(instancePath);
}

Ref< Instance > Database_createInstance(Database* self, const std::wstring& instancePath)
{
	return self->createInstance(instancePath);
}

Ref< ISerializable > Database_getObjectReadOnly(Database* self, const Guid& id)
{
	return self->getObjectReadOnly(id);
}

RefArray< Instance > Database_findInstancesByType(Database* self, const TypeInfo& instanceType)
{
	RefArray< Instance > instances;
	recursiveFindChildInstances(self->getRootGroup(), FindInstanceByType(instanceType), instances);
	return instances;
}

Ref< Instance > Group_getInstanceByName(Group* self, const std::wstring& instanceName)
{
	return self->getInstance(instanceName);
}

Ref< Instance > Group_createInstance(Group* self, const std::wstring& instancePath)
{
	return self->createInstance(instancePath);
}

RefArray< Group > Group_getChildGroups(Group* self)
{
	RefArray< Group > childGroups;
	self->getChildGroups(childGroups);
	return childGroups;
}

RefArray< Instance > Group_getChildInstances(Group* self)
{
	RefArray< Instance > childInstances;
	self->getChildInstances(childInstances);
	return childInstances;
}

void Instance_setName(Instance* self, const std::wstring& name)
{
	self->setName(name);
}

std::wstring Instance_getName(Instance* self)
{
	return self->getName();
}

void Instance_setGuid(Instance* self, const Guid& id)
{
	self->setGuid(id);
}

Guid Instance_getGuid(Instance* self)
{
	return self->getGuid();
}

Any Instance_getPrimaryType(Instance* self)
{
	const TypeInfo* primaryType = self->getPrimaryType();
	return primaryType ? CastAny< TypeInfo, false >::set(*primaryType) : Any();
}

void Instance_setObject(Instance* self, ISerializable* object)
{
	self->setObject(object);
}

Ref< ISerializable > Instance_getObject(Instance* self)
{
	return self->getObject();
}

std::vector< std::wstring > Instance_getDataNames(Instance* self)
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
	classDatabase->addProperty("rootGroup", &Database::getRootGroup);
	classDatabase->addMethod("close", &Database::close);
	classDatabase->addMethod("getGroup", &Database::getGroup);
	classDatabase->addMethod("createGroup", &Database::createGroup);
	classDatabase->addMethod("getInstanceByGuid", &Database_getInstanceByGuid);
	classDatabase->addMethod("getInstanceByPath", &Database_getInstanceByPath);
	classDatabase->addMethod("createInstance", &Database_createInstance);
	classDatabase->addMethod("getObjectReadOnly", &Database_getObjectReadOnly);
	classDatabase->addMethod("findInstancesByType", &Database_findInstancesByType);
	registrar->registerClass(classDatabase);

	Ref< AutoRuntimeClass< Group > > classGroup = new AutoRuntimeClass< Group >();
	classGroup->addProperty("name", &Group::getName);
	classGroup->addProperty("path", &Group::getPath);
	classGroup->addProperty("parent", &Group::getParent);
	classGroup->addProperty("childGroups", &Group_getChildGroups);
	classGroup->addProperty("childInstances", &Group_getChildInstances);
	classGroup->addMethod("rename", &Group::rename);
	classGroup->addMethod("remove", &Group::remove);
	classGroup->addMethod("getGroup", &Group::getGroup);
	classGroup->addMethod("createGroup", &Group::createGroup);
	classGroup->addMethod("getInstanceByName", &Group_getInstanceByName);
	classGroup->addMethod("createInstance", &Group_createInstance);
	registrar->registerClass(classGroup);

	Ref< AutoRuntimeClass< Instance > > classInstance = new AutoRuntimeClass< Instance >();
	classInstance->addProperty("parent", &Instance::getParent);
	classInstance->addProperty("name", &Instance_setName, &Instance_getName);
	classInstance->addProperty("path", &Instance::getPath);
	classInstance->addProperty("guid", &Instance_setGuid, &Instance_getGuid);
	classInstance->addProperty("primaryTypeName", &Instance::getPrimaryTypeName);
	classInstance->addProperty("primaryType", &Instance_getPrimaryType);
	classInstance->addProperty("object", &Instance_setObject, &Instance_getObject);
	classInstance->addProperty("dataNames", &Instance_getDataNames);
	classInstance->addMethod("readData", &Instance::readData);
	classInstance->addMethod("checkout", &Instance::checkout);
	classInstance->addMethod("commit", &Instance::commit);
	classInstance->addMethod("revert", &Instance::revert);
	classInstance->addMethod("remove", &Instance::remove);
	classInstance->addMethod("removeAllData", &Instance::removeAllData);
	classInstance->addMethod("writeData", &Instance::writeData);
	registrar->registerClass(classInstance);
}

	}
}
