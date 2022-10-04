#include "Avalanche/Server/Server.h"
#include "Avalanche/Server/ServerClassFactory.h"
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Settings/PropertyGroup.h"

namespace traktor::avalanche
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.avalanche.ServerClassFactory", 0, ServerClassFactory, IRuntimeClassFactory)

void ServerClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classServer = new AutoRuntimeClass< Server >();
	classServer->addConstructor();
	classServer->addMethod("create", &Server::create);
	classServer->addMethod("destroy", &Server::destroy);
	classServer->addMethod("update", &Server::update);
	registrar->registerClass(classServer);
}

}