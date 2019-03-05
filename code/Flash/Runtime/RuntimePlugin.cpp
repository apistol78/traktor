#include "Runtime/IEnvironment.h"
#include "Flash/MovieResourceFactory.h"
#include "Flash/Runtime/RuntimePlugin.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.RuntimePlugin", 0, RuntimePlugin, runtime::IRuntimePlugin)

bool RuntimePlugin::create(runtime::IEnvironment* environment)
{
	auto resourceManager = environment->getResource()->getResourceManager();
	
	resourceManager->addFactory(new flash::MovieResourceFactory());

	return true;
}

void RuntimePlugin::destroy(runtime::IEnvironment* environment)
{
}

Ref< runtime::IState > RuntimePlugin::createInitialState(runtime::IEnvironment* environment)
{
	return nullptr;
}

	}
}
