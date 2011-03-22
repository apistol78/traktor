#include "Scene/ISceneController.h"
#include "Scene/Scene.h"
#include "World/IWorldRenderer.h"
#include "World/WorldRenderSettings.h"
#include "World/Entity/Entity.h"
#include "World/Entity/EntityUpdate.h"

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
	world::PostProcessSettings* postProcessSettings
)
:	m_entitySchema(entitySchema)
,	m_rootEntity(rootEntity)
,	m_controller(controller)
,	m_worldRenderSettings(worldRenderSettings)
,	m_postProcessSettings(postProcessSettings)
{
}

Scene::~Scene()
{
	m_rootEntity = 0;
	m_entitySchema = 0;
	m_controller = 0;
	m_worldRenderSettings = 0;
	m_postProcessSettings = 0;
}

void Scene::destroy()
{
	if (m_rootEntity)
	{
		m_rootEntity->destroy();
		m_rootEntity = 0;
	}

	m_entitySchema = 0;
	m_controller = 0;
	m_worldRenderSettings = 0;
	m_postProcessSettings = 0;
}

void Scene::update(float time, float deltaTime, bool updateController)
{
	if (m_controller && updateController)
		m_controller->update(this, time, deltaTime);

	if (m_rootEntity)
	{
		world::EntityUpdate entityUpdate(deltaTime);
		m_rootEntity->update(&entityUpdate);
	}
}

void Scene::build(world::IWorldRenderer* worldRenderer, world::WorldRenderView& worldRenderView, int frame)
{
	if (m_rootEntity)
		worldRenderer->build(worldRenderView, m_rootEntity, frame);
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

world::PostProcessSettings* Scene::getPostProcessSettings() const
{
	return m_postProcessSettings;
}

	}
}
