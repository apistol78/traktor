#include "Amalgam/Run/IEnvironment.h"
#include "Amalgam/Run/RunClassFactory.h"
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Settings/PropertyGroup.h"
#include "Database/Database.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.RunClassFactory", 0, RunClassFactory, IRuntimeClassFactory)

void RunClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< IEnvironment > > classEnvironment = new AutoRuntimeClass< IEnvironment >();
	classEnvironment->addMethod("getDatabase", &IEnvironment::getDatabase);
	classEnvironment->addMethod("getScript", &IEnvironment::getScript);
	classEnvironment->addMethod("getSettings", &IEnvironment::getSettings);
	classEnvironment->addMethod("alive", &IEnvironment::alive);
	registrar->registerClass(classEnvironment);
}

	}
}
