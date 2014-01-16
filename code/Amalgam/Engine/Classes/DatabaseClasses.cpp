#include "Amalgam/Engine/Classes/DatabaseClasses.h"
#include "Core/Io/IStream.h"
#include "Core/Serialization/ISerializable.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Script/AutoScriptClass.h"
#include "Script/Boxes.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

Ref< db::Instance > db_Database_getInstanceByGuid(db::Database* self, const Guid& instanceGuid)
{
	return self->getInstance(instanceGuid);
}

Ref< db::Instance > db_Database_getInstanceByPath(db::Database* self, const std::wstring& instancePath)
{
	return self->getInstance(instancePath);
}

Ref< db::Instance > db_Database_createInstance(db::Database* self, const std::wstring& instancePath)
{
	return self->createInstance(instancePath);
}

Ref< ISerializable > db_Database_getObjectReadOnly(db::Database* self, const Guid& id)
{
	return self->getObjectReadOnly(id);
}

RefArray< db::Instance > db_Database_findInstancesByType(db::Database* self, const TypeInfo& instanceType)
{
	RefArray< db::Instance > instances;
	recursiveFindChildInstances(self->getRootGroup(), db::FindInstanceByType(instanceType), instances);
	return instances;
}

Ref< db::Instance > db_Group_getInstanceByName(db::Group* self, const std::wstring& instanceName)
{
	return self->getInstance(instanceName);
}

Ref< db::Instance > db_Group_createInstance(db::Group* self, const std::wstring& instancePath)
{
	return self->createInstance(instancePath);
}

RefArray< db::Group > db_Group_getChildGroups(db::Group* self)
{
	RefArray< db::Group > childGroups;
	self->getChildGroups(childGroups);
	return childGroups;
}

RefArray< db::Instance > db_Group_getChildInstances(db::Group* self)
{
	RefArray< db::Instance > childInstances;
	self->getChildInstances(childInstances);
	return childInstances;
}

Ref< ISerializable > db_Instance_getObject(db::Instance* self)
{
	return self->getObject();
}

std::vector< std::wstring > db_Instance_getDataNames(db::Instance* self)
{
	std::vector< std::wstring > dataNames;
	self->getDataNames(dataNames);
	return dataNames;
}

		}

void registerDatabaseClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< db::Database > > classDatabase = new script::AutoScriptClass< db::Database >();
	classDatabase->addMethod("close", &db::Database::close);
	classDatabase->addMethod("getRootGroup", &db::Database::getRootGroup);
	classDatabase->addMethod("getGroup", &db::Database::getGroup);
	classDatabase->addMethod("createGroup", &db::Database::createGroup);
	classDatabase->addMethod("getInstanceByGuid", &db_Database_getInstanceByGuid);
	classDatabase->addMethod("getInstanceByPath", &db_Database_getInstanceByPath);
	classDatabase->addMethod("createInstance", &db_Database_createInstance);
	classDatabase->addMethod("getObjectReadOnly", &db_Database_getObjectReadOnly);
	classDatabase->addMethod("findInstancesByType", &db_Database_findInstancesByType);
	scriptManager->registerClass(classDatabase);

	Ref< script::AutoScriptClass< db::Group > > classGroup = new script::AutoScriptClass< db::Group >();
	classGroup->addMethod("getName", &db::Group::getName);
	classGroup->addMethod("getPath", &db::Group::getPath);
	classGroup->addMethod("rename", &db::Group::rename);
	classGroup->addMethod("remove", &db::Group::remove);
	classGroup->addMethod("getGroup", &db::Group::getGroup);
	classGroup->addMethod("createGroup", &db::Group::createGroup);
	classGroup->addMethod("getInstanceByName", &db_Group_getInstanceByName);
	classGroup->addMethod("createInstance", &db_Group_createInstance);
	classGroup->addMethod("getParent", &db::Group::getParent);
	classGroup->addMethod("getChildGroups", &db_Group_getChildGroups);
	classGroup->addMethod("getChildInstances", &db_Group_getChildInstances);
	scriptManager->registerClass(classGroup);

	Ref< script::AutoScriptClass< db::Instance > > classInstance = new script::AutoScriptClass< db::Instance >();
	classInstance->addMethod("getParent", &db::Instance::getParent);
	classInstance->addMethod("getName", &db::Instance::getName);
	classInstance->addMethod("getPath", &db::Instance::getPath);
	classInstance->addMethod("getGuid", &db::Instance::getGuid);
	classInstance->addMethod("getPrimaryTypeName", &db::Instance::getPrimaryTypeName);
	classInstance->addMethod("getPrimaryType", &db::Instance::getPrimaryType);
	classInstance->addMethod("getObject", &db_Instance_getObject);
	classInstance->addMethod("getDataNames", &db_Instance_getDataNames);
	classInstance->addMethod("readData", &db::Instance::readData);
	classInstance->addMethod("checkout", &db::Instance::checkout);
	classInstance->addMethod("commit", &db::Instance::commit);
	classInstance->addMethod("revert", &db::Instance::revert);
	classInstance->addMethod("remove", &db::Instance::remove);
	classInstance->addMethod("setName", &db::Instance::setName);
	classInstance->addMethod("setGuid", &db::Instance::setGuid);
	classInstance->addMethod("setObject", &db::Instance::setObject);
	classInstance->addMethod("removeAllData", &db::Instance::removeAllData);
	classInstance->addMethod("writeData", &db::Instance::writeData);
	scriptManager->registerClass(classInstance);
}

	}
}
