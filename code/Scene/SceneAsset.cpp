#include "Scene/SceneAsset.h"
#include "Scene/Scene.h"
#include "Scene/ISceneControllerData.h"
#include "World/WorldRenderSettings.h"
#include "World/Entity/EntityInstance.h"
#include "World/Entity/IEntityBuilder.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.scene.SceneAsset", SceneAsset, Serializable)

SceneAsset::SceneAsset()
:	m_worldRenderSettings(gc_new< world::WorldRenderSettings >())
{
}

Scene* SceneAsset::createScene(world::IEntityBuilder* entityBuilder, world::IEntityManager* entityManager) const
{
	entityBuilder->begin(entityManager);

	Ref< world::Entity > rootEntity = entityBuilder->build(m_instance);

	Ref< ISceneController > controller;
	if (m_controllerData)
	{
		controller = m_controllerData->createController(entityBuilder, entityManager);
		if (!controller)
			return 0;
	}

	entityBuilder->end();

	return gc_new< Scene >(
		controller,
		entityManager,
		rootEntity,
		m_worldRenderSettings
	);
}

void SceneAsset::setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings)
{
	m_worldRenderSettings = worldRenderSettings;
}

world::WorldRenderSettings* SceneAsset::getWorldRenderSettings() const
{
	return m_worldRenderSettings;
}

void SceneAsset::setInstance(world::EntityInstance* instance)
{
	m_instance = instance;
}

world::EntityInstance* SceneAsset::getInstance() const
{
	return m_instance;
}

void SceneAsset::setControllerData(ISceneControllerData* controllerData)
{
	m_controllerData = controllerData;
}

ISceneControllerData* SceneAsset::getControllerData() const
{
	return m_controllerData;
}

int SceneAsset::getVersion() const
{
	return 1;
}

bool SceneAsset::serialize(Serializer& s)
{
	s >> MemberRef< world::WorldRenderSettings >(L"worldRenderSettings", m_worldRenderSettings);
	s >> MemberRef< world::EntityInstance >(L"instance", m_instance);

	if (s.getVersion() >= 1)
	{
		s >> MemberRef< ISceneControllerData >(L"controllerData", m_controllerData);
	}

	return true;
}

	}
}
