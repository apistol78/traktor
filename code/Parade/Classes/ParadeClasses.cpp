#include "Parade/AudioLayer.h"
#include "Parade/Stage.h"
#include "Parade/StageLoader.h"
#include "Parade/VideoLayer.h"
#include "Parade/WorldLayer.h"
#include "Parade/Classes/ParadeClasses.h"
#include "Script/AutoScriptClass.h"
#include "Script/Boxes.h"
#include "Script/Delegate.h"
#include "Script/IScriptManager.h"
#include "World/Entity/IEntitySchema.h"
#include "World/Entity/Entity.h"

namespace traktor
{
	namespace parade
	{
		namespace
		{

Ref< world::Entity > WorldLayer_createEntity1(WorldLayer* self, const std::wstring& name, world::IEntitySchema* entitySchema)
{
	return self->createEntity(name, entitySchema);
}

Ref< world::Entity > WorldLayer_createEntity2(WorldLayer* self, const std::wstring& name)
{
	return self->createEntity(name, 0);
}

Ref< script::BoxedVector4 > WorldLayer_getViewPosition(WorldLayer* self, const Vector4& worldPosition)
{
	Vector4 viewPosition;
	if (self->getViewPosition(worldPosition, viewPosition))
		return new script::BoxedVector4(viewPosition);
	else
		return 0;
}

Ref< script::BoxedVector2 > WorldLayer_getScreenPosition(WorldLayer* self, const Vector4& viewPosition)
{
	Vector2 screenPosition;
	if (self->getScreenPosition(viewPosition, screenPosition))
		return new script::BoxedVector2(screenPosition);
	else
		return 0;
}

	}

void registerParadeClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< Stage > > classStage = new script::AutoScriptClass< Stage >();
	classStage->addMethod(L"addLayer", &Stage::addLayer);
	classStage->addMethod(L"removeLayer", &Stage::removeLayer);
	classStage->addMethod(L"removeAllLayers", &Stage::removeAllLayers);
	classStage->addMethod(L"findLayer", &Stage::findLayer);
	classStage->addMethod(L"terminate", &Stage::terminate);
	classStage->addMethod(L"loadStage", &Stage::loadStage);
	classStage->addMethod(L"loadStageAsync", &Stage::loadStageAsync);
	classStage->addMethod(L"gotoStage", &Stage::gotoStage);
	classStage->addMethod(L"getEnvironment", &Stage::getEnvironment);
	classStage->addMethod(L"getLayers", &Stage::getLayers);
	scriptManager->registerClass(classStage);

	Ref< script::AutoScriptClass< Layer > > classLayer = new script::AutoScriptClass< Layer >();
	classLayer->addMethod(L"getName", &Layer::getName);
	scriptManager->registerClass(classLayer);

	Ref< script::AutoScriptClass< AudioLayer > > classAudioLayer = new script::AutoScriptClass< AudioLayer >();
	scriptManager->registerClass(classAudioLayer);

	Ref< script::AutoScriptClass< VideoLayer > > classVideoLayer = new script::AutoScriptClass< VideoLayer >();
	classVideoLayer->addMethod(L"playing", &VideoLayer::playing);
	scriptManager->registerClass(classVideoLayer);

	Ref< script::AutoScriptClass< WorldLayer > > classWorldLayer = new script::AutoScriptClass< WorldLayer >();
	classWorldLayer->addMethod(L"getEntity", &WorldLayer::getEntity);
	classWorldLayer->addMethod(L"getEntities", &WorldLayer::getEntities);
	classWorldLayer->addMethod(L"getEntitiesOf", &WorldLayer::getEntitiesOf);
	classWorldLayer->addMethod(L"createEntity", &WorldLayer_createEntity1);
	classWorldLayer->addMethod(L"createEntity", &WorldLayer_createEntity2);
	classWorldLayer->addMethod(L"getEntityIndex", &WorldLayer::getEntityIndex);
	classWorldLayer->addMethod(L"getEntityIndexOf", &WorldLayer::getEntityIndexOf);
	classWorldLayer->addMethod(L"getEntityByIndex", &WorldLayer::getEntityByIndex);
	classWorldLayer->addMethod(L"getEntityOf", &WorldLayer::getEntityOf);
	classWorldLayer->addMethod(L"addEntity", &WorldLayer::addEntity);
	classWorldLayer->addMethod(L"addTransientEntity", &WorldLayer::addTransientEntity);
	classWorldLayer->addMethod(L"removeEntity", &WorldLayer::removeEntity);
	classWorldLayer->addMethod(L"getEntitySchema", &WorldLayer::getEntitySchema);
	classWorldLayer->addMethod(L"setControllerEnable", &WorldLayer::setControllerEnable);
	classWorldLayer->addMethod(L"getPostProcess", &WorldLayer::getPostProcess);
	classWorldLayer->addMethod(L"getViewPosition", &WorldLayer_getViewPosition);
	classWorldLayer->addMethod(L"getScreenPosition", &WorldLayer_getScreenPosition);
	classWorldLayer->addMethod(L"setFieldOfView", &WorldLayer::setFieldOfView);
	classWorldLayer->addMethod(L"getFieldOfView", &WorldLayer::getFieldOfView);
	classWorldLayer->addMethod(L"setAlternateTime", &WorldLayer::setAlternateTime);
	classWorldLayer->addMethod(L"getAlternateTime", &WorldLayer::getAlternateTime);
	scriptManager->registerClass(classWorldLayer);

	Ref< script::AutoScriptClass< StageLoader > > classStageLoader = new script::AutoScriptClass< StageLoader >();
	classStageLoader->addMethod(L"wait", &StageLoader::wait);
	classStageLoader->addMethod(L"ready", &StageLoader::ready);
	classStageLoader->addMethod(L"succeeded", &StageLoader::succeeded);
	classStageLoader->addMethod(L"failed", &StageLoader::failed);
	classStageLoader->addMethod(L"get", &StageLoader::get);
	scriptManager->registerClass(classStageLoader);
}

	}
}
