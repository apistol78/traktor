#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Remote/Server/Server.h"
#include "Remote/Server/ServerClassFactory.h"

namespace traktor
{
	namespace remote
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.remote.ServerClassFactory", 0, ServerClassFactory, IRuntimeClassFactory)

void ServerClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classServer = new AutoRuntimeClass< Server >();
	classServer->addConstructor();
	classServer->addProperty("listenPort", &Server::getListenPort);
	classServer->addProperty("scratchPath", &Server::getScratchPath);
	classServer->addMethod("create", &Server::create);
	classServer->addMethod("destroy", &Server::destroy);
	classServer->addMethod("update", &Server::update);
	registrar->registerClass(classServer);
}

	}
}