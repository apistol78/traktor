#include "Scene/SceneAsset.h"
#include "Scene/Scene.h"
#include "Scene/ISceneControllerData.h"
#include "World/WorldRenderSettings.h"
#include "World/Entity/EntityInstance.h"
#include "World/Entity/IEntityBuilder.h"
#include "World/PostProcess/PostProcessSettings.h"
#include "World/PostProcess/PostProcess.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.scene.SceneAsset", SceneAsset, Serializable)

SceneAsset::SceneAsset()
:	m_worldRenderSettings(gc_new< world::WorldRenderSettings >())
{
}

Scene* SceneAsset::createScene(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	world::IEntityBuilder* entityBuilder,
	world::IEntityManager* entityManager
) const
{
	Ref< world::PostProcess > postProcess;

	if (!m_postProcessSettings.getGuid().isNull())
	{
		if (!resourceManager->bind(m_postProcessSettings))
			return 0;

		postProcess = gc_new< world::PostProcess >();
		if (!postProcess->create(m_postProcessSettings, resourceManager, renderSystem))
			return 0;
	}

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
		m_worldRenderSettings,
		postProcess
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

void SceneAsset::setPostProcessSettings(const resource::Proxy< world::PostProcessSettings >& postProcessSettings)
{
	m_postProcessSettings = postProcessSettings;
}

const resource::Proxy< world::PostProcessSettings >& SceneAsset::getPostProcessSettings() const
{
	return m_postProcessSettings;
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
	return 2;
}

bool SceneAsset::serialize(Serializer& s)
{
	s >> MemberRef< world::WorldRenderSettings >(L"worldRenderSettings", m_worldRenderSettings);

	if (s.getVersion() >= 2)
	{
		s >> resource::Member< world::PostProcessSettings >(L"postProcessSettings", m_postProcessSettings);
	}

	s >> MemberRef< world::EntityInstance >(L"instance", m_instance);

	if (s.getVersion() >= 1)
	{
		s >> MemberRef< ISceneControllerData >(L"controllerData", m_controllerData);
	}

	return true;
}

	}
}
