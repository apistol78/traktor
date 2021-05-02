#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Scene/ISceneController.h"
#include "Scene/Scene.h"
#include "Scene/SceneClassFactory.h"
#include "World/Entity.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.SceneClassFactory", 0, SceneClassFactory, IRuntimeClassFactory)

void SceneClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classISceneController = new AutoRuntimeClass< ISceneController >();
	registrar->registerClass(classISceneController);

	auto classScene = new AutoRuntimeClass< Scene >();
	classScene->addProperty("rootEntity", &Scene::getRootEntity);
	classScene->addProperty("controller", &Scene::getController);
	registrar->registerClass(classScene);
}

	}
}
