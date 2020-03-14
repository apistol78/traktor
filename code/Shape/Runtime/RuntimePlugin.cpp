#include "Runtime/IEnvironment.h"
#include "Shape/EntityFactory.h"
#include "Shape/EntityRenderer.h"
#include "Shape/Runtime/RuntimePlugin.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.RuntimePlugin", 0, RuntimePlugin, runtime::IRuntimePlugin)

bool RuntimePlugin::create(runtime::IEnvironment* environment)
{
	auto resourceManager = environment->getResource()->getResourceManager();
	auto renderSystem = environment->getRender()->getRenderSystem();
	auto worldServer = environment->getWorld();

	worldServer->addEntityFactory(new EntityFactory(resourceManager, renderSystem));
	worldServer->addEntityRenderer(new EntityRenderer());
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
