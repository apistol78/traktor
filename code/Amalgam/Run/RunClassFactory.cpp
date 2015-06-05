#include "Amalgam/Run/IEnvironment.h"
#include "Amalgam/Run/RunClassFactory.h"
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.RunClassFactory", 0, RunClassFactory, IRuntimeClassFactory)

void RunClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< amalgam::IEnvironment > > classEnvironment = new AutoRuntimeClass< amalgam::IEnvironment >();
	classEnvironment->addMethod("getDatabase", &amalgam::IEnvironment::getDatabase);
	classEnvironment->addMethod("getScript", &amalgam::IEnvironment::getScript);
	classEnvironment->addMethod("getSettings", &amalgam::IEnvironment::getSettings);
	classEnvironment->addMethod("alive", &amalgam::IEnvironment::alive);
	registrar->registerClass(classEnvironment);
}

	}
}
