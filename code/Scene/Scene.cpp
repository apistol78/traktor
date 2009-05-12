#include "Scene/Scene.h"
#include "World/Entity/Entity.h"
#include "World/WorldRenderSettings.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.Scene", Scene, Object)

Scene::Scene(
	world::EntityManager* entityManager,
	world::Entity* rootEntity,
	world::WorldRenderSettings* worldRenderSettings
)
:	m_entityManager(entityManager)
,	m_rootEntity(rootEntity)
,	m_worldRenderSettings(worldRenderSettings)
{
}

world::EntityManager* Scene::getEntityManager() const
{
	return m_entityManager;
}

world::Entity* Scene::getRootEntity() const
{
	return m_rootEntity;
}

world::WorldRenderSettings* Scene::getWorldRenderSettings() const
{
	return m_worldRenderSettings;
}

	}
}
