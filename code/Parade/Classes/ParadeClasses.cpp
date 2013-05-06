#include "Parade/AudioLayer.h"
#include "Parade/Stage.h"
#include "Parade/StageData.h"
#include "Parade/StageLoader.h"
#include "Parade/VideoLayer.h"
#include "Parade/WorldLayer.h"
#include "Parade/Classes/ParadeClasses.h"
#include "Script/AutoScriptClass.h"
#include "Script/Boxes.h"
#include "Script/Delegate.h"
#include "Script/IScriptManager.h"
#include "World/Entity.h"
#include "World/EntityData.h"
#include "World/IEntitySchema.h"

namespace traktor
{
	namespace parade
	{
		namespace
		{

class BoxedTransition : public Object
{
	T_RTTI_CLASS;

public:
	BoxedTransition(const std::wstring& id, const Guid& reference)
	:	m_id(id)
	,	m_reference(reference)
	{
	}

	const std::wstring& getId() const { return m_id; }

	const Guid& getReference() const { return m_reference; }

private:
	std::wstring m_id;
	Guid m_reference;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.Transition", BoxedTransition, Object)

RefArray< BoxedTransition > StageData_getTransitions(StageData* self)
{
	RefArray< BoxedTransition > out;

	const std::map< std::wstring, Guid >& transitions = self->getTransitions();
	for (std::map< std::wstring, Guid >::const_iterator i = transitions.begin(); i != transitions.end(); ++i)
		out.push_back(new BoxedTransition(i->first, i->second));

	return out;
}

Ref< world::Entity > WorldLayer_createEntity1(WorldLayer* self, const std::wstring& name, world::IEntitySchema* entitySchema)
{
	return self->createEntity(name, entitySchema);
}

Ref< world::Entity > WorldLayer_createEntity2(WorldLayer* self, const std::wstring& name)
{
	return self->createEntity(name, 0);
}

Ref< script::BoxedVector4 > WorldLayer_worldToView(WorldLayer* self, const Vector4& worldPosition)
{
	Vector4 viewPosition;
	if (self->worldToView(worldPosition, viewPosition))
		return new script::BoxedVector4(viewPosition);
	else
		return 0;
}

Ref< script::BoxedVector4 > WorldLayer_viewToWorld(WorldLayer* self, const Vector4& viewPosition)
{
	Vector4 worldPosition;
	if (self->viewToWorld(viewPosition, worldPosition))
		return new script::BoxedVector4(worldPosition);
	else
		return 0;
}

Ref< script::BoxedVector2 > WorldLayer_worldToScreen(WorldLayer* self, const Vector4& worldPosition)
{
	Vector2 screenPosition;
	if (self->worldToScreen(worldPosition, screenPosition))
		return new script::BoxedVector2(screenPosition);
	else
		return 0;
}

Ref< script::BoxedVector2 > WorldLayer_viewToScreen(WorldLayer* self, const Vector4& viewPosition)
{
	Vector2 screenPosition;
	if (self->viewToScreen(viewPosition, screenPosition))
		return new script::BoxedVector2(screenPosition);
	else
		return 0;
}

	}

void registerParadeClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< BoxedTransition > > classBoxedTransition = new script::AutoScriptClass< BoxedTransition >();
	classBoxedTransition->addMethod("getId", &BoxedTransition::getId);
	classBoxedTransition->addMethod("getReference", &BoxedTransition::getReference);
	scriptManager->registerClass(classBoxedTransition);

	Ref< script::AutoScriptClass< StageData > > classStageData = new script::AutoScriptClass< StageData >();
	classStageData->addMethod("getTransitions", &StageData_getTransitions);
	scriptManager->registerClass(classStageData);

	Ref< script::AutoScriptClass< Stage > > classStage = new script::AutoScriptClass< Stage >();
	classStage->addMethod("addLayer", &Stage::addLayer);
	classStage->addMethod("removeLayer", &Stage::removeLayer);
	classStage->addMethod("removeAllLayers", &Stage::removeAllLayers);
	classStage->addMethod("findLayer", &Stage::findLayer);
	classStage->addMethod("terminate", &Stage::terminate);
	classStage->addMethod("loadStage", &Stage::loadStage);
	classStage->addMethod("loadStageAsync", &Stage::loadStageAsync);
	classStage->addMethod("gotoStage", &Stage::gotoStage);
	classStage->addMethod("getEnvironment", &Stage::getEnvironment);
	classStage->addMethod("getLayers", &Stage::getLayers);
	scriptManager->registerClass(classStage);

	Ref< script::AutoScriptClass< Layer > > classLayer = new script::AutoScriptClass< Layer >();
	classLayer->addMethod("getName", &Layer::getName);
	scriptManager->registerClass(classLayer);

	Ref< script::AutoScriptClass< AudioLayer > > classAudioLayer = new script::AutoScriptClass< AudioLayer >();
	classAudioLayer->addMethod("play", &AudioLayer::play);
	classAudioLayer->addMethod("stop", &AudioLayer::stop);
	classAudioLayer->addMethod("fadeOff", &AudioLayer::fadeOff);
	classAudioLayer->addMethod("setParameter", &AudioLayer::setParameter);
	classAudioLayer->addMethod("tweenParameter", &AudioLayer::tweenParameter);
	scriptManager->registerClass(classAudioLayer);

	Ref< script::AutoScriptClass< VideoLayer > > classVideoLayer = new script::AutoScriptClass< VideoLayer >();
	classVideoLayer->addMethod("playing", &VideoLayer::playing);
	scriptManager->registerClass(classVideoLayer);

	Ref< script::AutoScriptClass< WorldLayer > > classWorldLayer = new script::AutoScriptClass< WorldLayer >();
	classWorldLayer->addMethod("getEntityData", &WorldLayer::getEntityData);
	classWorldLayer->addMethod("getEntity", &WorldLayer::getEntity);
	classWorldLayer->addMethod("getEntities", &WorldLayer::getEntities);
	classWorldLayer->addMethod("getEntitiesOf", &WorldLayer::getEntitiesOf);
	classWorldLayer->addMethod("createEntity", &WorldLayer_createEntity1);
	classWorldLayer->addMethod("createEntity", &WorldLayer_createEntity2);
	classWorldLayer->addMethod("getEntityIndex", &WorldLayer::getEntityIndex);
	classWorldLayer->addMethod("getEntityIndexOf", &WorldLayer::getEntityIndexOf);
	classWorldLayer->addMethod("getEntityByIndex", &WorldLayer::getEntityByIndex);
	classWorldLayer->addMethod("getEntityOf", &WorldLayer::getEntityOf);
	classWorldLayer->addMethod("addEntity", &WorldLayer::addEntity);
	classWorldLayer->addMethod("addTransientEntity", &WorldLayer::addTransientEntity);
	classWorldLayer->addMethod("removeEntity", &WorldLayer::removeEntity);
	classWorldLayer->addMethod("getEntitySchema", &WorldLayer::getEntitySchema);
	classWorldLayer->addMethod("setControllerEnable", &WorldLayer::setControllerEnable);
	classWorldLayer->addMethod("getPostProcess", &WorldLayer::getPostProcess);
	classWorldLayer->addMethod("worldToView", &WorldLayer_worldToView);
	classWorldLayer->addMethod("viewToWorld", &WorldLayer_viewToWorld);
	classWorldLayer->addMethod("worldToScreen", &WorldLayer_worldToScreen);
	classWorldLayer->addMethod("viewToScreen", &WorldLayer_viewToScreen);
	classWorldLayer->addMethod("setFieldOfView", &WorldLayer::setFieldOfView);
	classWorldLayer->addMethod("getFieldOfView", &WorldLayer::getFieldOfView);
	classWorldLayer->addMethod("setAlternateTime", &WorldLayer::setAlternateTime);
	classWorldLayer->addMethod("getAlternateTime", &WorldLayer::getAlternateTime);
	scriptManager->registerClass(classWorldLayer);

	Ref< script::AutoScriptClass< StageLoader > > classStageLoader = new script::AutoScriptClass< StageLoader >();
	classStageLoader->addMethod("wait", &StageLoader::wait);
	classStageLoader->addMethod("ready", &StageLoader::ready);
	classStageLoader->addMethod("succeeded", &StageLoader::succeeded);
	classStageLoader->addMethod("failed", &StageLoader::failed);
	classStageLoader->addMethod("get", &StageLoader::get);
	scriptManager->registerClass(classStageLoader);
}

	}
}
