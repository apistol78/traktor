#include <Amalgam/Game/IEnvironment.h>
#include "Shared/EntityFactory.h"
#include "Shared/EntityRenderer.h"
#include "Shared/RuntimePlugin.h"

using namespace traktor;

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"RuntimePlugin", 0, RuntimePlugin, amalgam::IRuntimePlugin)

bool RuntimePlugin::getDependencies(TypeInfoSet& outDependencies) const
{
	return true;
}

bool RuntimePlugin::startup(amalgam::IEnvironment* environment)
{
	// Add entity factory to world system.
	environment->getWorld()->addEntityFactory(new EntityFactory(
		environment->getResource()->getResourceManager(),
		environment->getRender()->getRenderSystem()
	));

	// Add entity renderer to world system.
	environment->getWorld()->addEntityRenderer(new EntityRenderer());
	return true;
}

bool RuntimePlugin::shutdown(amalgam::IEnvironment* environment)
{
	return true;
}

Ref< amalgam::IState > RuntimePlugin::createInitialState(amalgam::IEnvironment* environment) const
{
	return 0;
}
