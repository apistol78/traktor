#include "Amalgam/Run/IEnvironment.h"
#include "Amalgam/Run/IResourceServer.h"
#include "Amalgam/Run/RunClassFactory.h"
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Settings/PropertyGroup.h"
#include "Database/Database.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.RunClassFactory", 0, RunClassFactory, IRuntimeClassFactory)

void RunClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classEnvironment = new AutoRuntimeClass< IEnvironment >();
	classEnvironment->addProperty< db::Database* >("database", &IEnvironment::getDatabase);
	classEnvironment->addProperty< IResourceServer* >("resource", &IEnvironment::getResource);
	classEnvironment->addProperty< IScriptServer* >("script", &IEnvironment::getScript);
	classEnvironment->addProperty< PropertyGroup* >("settings", &IEnvironment::getSettings);
	classEnvironment->addProperty< bool >("alive", &IEnvironment::alive);
	registrar->registerClass(classEnvironment);

	auto classResourceServer = new AutoRuntimeClass< IResourceServer >();
	classResourceServer->addProperty< resource::IResourceManager* >("resourceManager", 0, &IResourceServer::getResourceManager);
	registrar->registerClass(classResourceServer);
}

	}
}
