#include "Core/Misc/SafeDestroy.h"
#include "Scene/ISceneController.h"
#include "Scene/Scene.h"
#include "World/Entity.h"
#include "World/IWorldRenderer.h"
#include "World/WorldRenderSettings.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.Scene", Scene, Object)

Scene::Scene(
	ISceneController* controller,
	world::IEntitySchema* entitySchema,
	world::Entity* rootEntity,
	world::WorldRenderSettings* worldRenderSettings,
	const resource::Proxy< render::ImageProcessSettings > imageProcessSettings[world::QuLast],
	const SmallMap< render::handle_t, resource::Proxy< render::ITexture > >& imageProcessParams
)
:	m_entitySchema(entitySchema)
,	m_rootEntity(rootEntity)
,	m_controller(controller)
,	m_worldRenderSettings(worldRenderSettings)
,	m_imageProcessParams(imageProcessParams)
{
	for (int32_t i = 0; i < world::QuLast; ++i)
		m_imageProcessSettings[i] = imageProcessSettings[i];
}

Scene::Scene(ISceneController* controller, Scene* scene)
:	m_entitySchema(scene->m_entitySchema)
,	m_rootEntity(scene->m_rootEntity)
,	m_controller(controller)
,	m_worldRenderSettings(scene->m_worldRenderSettings)
,	m_imageProcessParams(scene->m_imageProcessParams)
{
	for (int32_t i = 0; i < world::QuLast; ++i)
		m_imageProcessSettings[i] = scene->m_imageProcessSettings[i];
}

Scene::~Scene()
{
	m_rootEntity = 0;
	m_entitySchema = 0;
	m_controller = 0;
	m_worldRenderSettings = 0;
	
	for (int32_t i = 0; i < world::QuLast; ++i)
		m_imageProcessSettings[i].clear();

	m_imageProcessParams.clear();
}

void Scene::destroy()
{
	safeDestroy(m_rootEntity);
	m_entitySchema = 0;
	m_controller = 0;
	m_worldRenderSettings = 0;

	for (int32_t i = 0; i < world::QuLast; ++i)
		m_imageProcessSettings[i].clear();

	m_imageProcessParams.clear();
}

void Scene::update(const world::UpdateParams& update, bool updateController, bool updateEntity)
{
	if (m_controller && updateController)
		m_controller->update(this, update.totalTime, update.deltaTime);

	if (m_rootEntity && updateEntity)
		m_rootEntity->update(update);
}

world::IEntitySchema* Scene::getEntitySchema() const
{
	return m_entitySchema;
}

world::Entity* Scene::getRootEntity() const
{
	return m_rootEntity;
}

ISceneController* Scene::getController() const
{
	return m_controller;
}

world::WorldRenderSettings* Scene::getWorldRenderSettings() const
{
	return m_worldRenderSettings;
}

const resource::Proxy< render::ImageProcessSettings >& Scene::getImageProcessSettings(world::Quality quality) const
{
	return m_imageProcessSettings[int32_t(quality)];
}

const SmallMap< render::handle_t, resource::Proxy< render::ITexture > >& Scene::getImageProcessParams() const
{
	return m_imageProcessParams;
}

	}
}
