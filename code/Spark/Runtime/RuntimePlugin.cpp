#include "Runtime/IEnvironment.h"
#include "Spark/GC.h"
#include "Spark/MovieResourceFactory.h"
#include "Spark/Runtime/RuntimePlugin.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.RuntimePlugin", 0, RuntimePlugin, runtime::IRuntimePlugin)

bool RuntimePlugin::create(runtime::IEnvironment* environment)
{
	auto resourceManager = environment->getResource()->getResourceManager();
	resourceManager->addFactory(new MovieResourceFactory());
	return true;
}

void RuntimePlugin::destroy(runtime::IEnvironment* environment)
{
	GC::getInstance().collectCycles(true);
}

Ref< runtime::IState > RuntimePlugin::createInitialState(runtime::IEnvironment* environment)
{
	return nullptr;
}

	}
}
