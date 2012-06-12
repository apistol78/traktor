#include "Core/Io/IStream.h"
#include "Core/Serialization/ISerializable.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Parade/Classes/DatabaseClasses.h"
#include "Script/AutoScriptClass.h"
#include "Script/Boxes.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace parade
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
	classDatabase->addMethod(L"close", &db::Database::close);
	classDatabase->addMethod(L"getRootGroup", &db::Database::getRootGroup);
	classDatabase->addMethod(L"getGroup", &db::Database::getGroup);
	classDatabase->addMethod(L"createGroup", &db::Database::createGroup);
	classDatabase->addMethod(L"getInstanceByGuid", &db_Database_getInstanceByGuid);
	classDatabase->addMethod(L"getInstanceByPath", &db_Database_getInstanceByPath);
	classDatabase->addMethod(L"createInstance", &db_Database_createInstance);
	classDatabase->addMethod(L"getObjectReadOnly", &db::Database::getObjectReadOnly);
	scriptManager->registerClass(classDatabase);

	Ref< script::AutoScriptClass< db::Group > > classGroup = new script::AutoScriptClass< db::Group >();
	classGroup->addMethod(L"getName", &db::Group::getName);
	classGroup->addMethod(L"getPath", &db::Group::getPath);
	classGroup->addMethod(L"rename", &db::Group::rename);
	classGroup->addMethod(L"remove", &db::Group::remove);
	classGroup->addMethod(L"getGroup", &db::Group::getGroup);
	classGroup->addMethod(L"createGroup", &db::Group::createGroup);
	classGroup->addMethod(L"getInstanceByName", &db_Group_getInstanceByName);
	classGroup->addMethod(L"createInstance", &db_Group_createInstance);
	classGroup->addMethod(L"getParent", &db::Group::getParent);
	classGroup->addMethod(L"getChildGroups", &db_Group_getChildGroups);
	classGroup->addMethod(L"getChildInstances", &db_Group_getChildInstances);
	scriptManager->registerClass(classGroup);

	Ref< script::AutoScriptClass< db::Instance > > classInstance = new script::AutoScriptClass< db::Instance >();
	classInstance->addMethod(L"getParent", &db::Instance::getParent);
	classInstance->addMethod(L"getName", &db::Instance::getName);
	classInstance->addMethod(L"getPath", &db::Instance::getPath);
	classInstance->addMethod(L"getGuid", &db::Instance::getGuid);
	classInstance->addMethod(L"getPrimaryTypeName", &db::Instance::getPrimaryTypeName);
	classInstance->addMethod(L"getPrimaryType", &db::Instance::getPrimaryType);
	classInstance->addMethod(L"getObject", &db::Instance::getObject);
	classInstance->addMethod(L"getDataNames", &db_Instance_getDataNames);
	classInstance->addMethod(L"readData", &db::Instance::readData);
	classInstance->addMethod(L"checkout", &db::Instance::checkout);
	classInstance->addMethod(L"commit", &db::Instance::commit);
	classInstance->addMethod(L"revert", &db::Instance::revert);
	classInstance->addMethod(L"remove", &db::Instance::remove);
	classInstance->addMethod(L"setName", &db::Instance::setName);
	classInstance->addMethod(L"setGuid", &db::Instance::setGuid);
	classInstance->addMethod(L"setObject", &db::Instance::setObject);
	classInstance->addMethod(L"removeAllData", &db::Instance::removeAllData);
	classInstance->addMethod(L"writeData", &db::Instance::writeData);
	scriptManager->registerClass(classInstance);
}

	}
}
