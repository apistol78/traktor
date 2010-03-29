#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Scene/ISceneControllerData.h"
#include "Scene/Scene.h"
#include "Scene/SceneResource.h"
#include "World/WorldRenderSettings.h"
#include "World/Entity/EntityInstance.h"
#include "World/Entity/IEntityBuilder.h"
#include "World/PostProcess/PostProcessSettings.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.SceneResource", 0, SceneResource, ISerializable)

SceneResource::SceneResource()
:	m_worldRenderSettings(new world::WorldRenderSettings())
{
}

Ref< Scene > SceneResource::createScene(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	world::IEntityBuilder* entityBuilder,
	world::IEntityManager* entityManager,
	world::WorldRenderSettings::ShadowQuality shadowQuality
) const
{
	if (!m_postProcessSettings.getGuid().isNull())
	{
		if (!resourceManager->bind(m_postProcessSettings))
			log::error << L"Unable to bind post processing settings" << Endl;
		if (!m_postProcessSettings.validate())
			log::error << L"Unable to validate post processing settings" << Endl;
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

	Ref< world::WorldRenderSettings > worldRenderSettings = new world::WorldRenderSettings(*m_worldRenderSettings);
	worldRenderSettings->shadowsQuality = min(m_worldRenderSettings->shadowsQuality, shadowQuality);

	return new Scene(
		controller,
		entityManager,
		rootEntity,
		worldRenderSettings,
		m_postProcessSettings
	);
}

void SceneResource::setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings)
{
	m_worldRenderSettings = worldRenderSettings;
}

Ref< world::WorldRenderSettings > SceneResource::getWorldRenderSettings() const
{
	return m_worldRenderSettings;
}

void SceneResource::setPostProcessSettings(const resource::Proxy< world::PostProcessSettings >& postProcessSettings)
{
	m_postProcessSettings = postProcessSettings;
}

const resource::Proxy< world::PostProcessSettings >& SceneResource::getPostProcessSettings() const
{
	return m_postProcessSettings;
}

void SceneResource::setInstance(world::EntityInstance* instance)
{
	m_instance = instance;
}

Ref< world::EntityInstance > SceneResource::getInstance() const
{
	return m_instance;
}

void SceneResource::setControllerData(ISceneControllerData* controllerData)
{
	m_controllerData = controllerData;
}

Ref< ISceneControllerData > SceneResource::getControllerData() const
{
	return m_controllerData;
}

bool SceneResource::serialize(ISerializer& s)
{
	s >> MemberRef< world::WorldRenderSettings >(L"worldRenderSettings", m_worldRenderSettings);
	s >> resource::Member< world::PostProcessSettings >(L"postProcessSettings", m_postProcessSettings);
	s >> MemberRef< world::EntityInstance >(L"instance", m_instance);
	s >> MemberRef< ISceneControllerData >(L"controllerData", m_controllerData);
	return true;
}

	}
}
