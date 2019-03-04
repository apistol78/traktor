#include "Amalgam/Game/IEnvironment.h"
#include "Flash/MovieResourceFactory.h"
#include "Flash/Runtime/RuntimePlugin.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.RuntimePlugin", 0, RuntimePlugin, amalgam::IRuntimePlugin)

bool RuntimePlugin::create(amalgam::IEnvironment* environment)
{
	auto resourceManager = environment->getResource()->getResourceManager();
	
	resourceManager->addFactory(new flash::MovieResourceFactory());

	return true;
}

void RuntimePlugin::destroy(amalgam::IEnvironment* environment)
{
}

Ref< amalgam::IState > RuntimePlugin::createInitialState(amalgam::IEnvironment* environment)
{
	return nullptr;
}

	}
}
