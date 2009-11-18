#include "Scene/SceneAsset.h"
#include "Scene/Scene.h"
#include "Scene/ISceneControllerData.h"
#include "World/WorldRenderSettings.h"
#include "World/Entity/EntityInstance.h"
#include "World/Entity/IEntityBuilder.h"
#include "World/PostProcess/PostProcessSettings.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.scene.SceneAsset", 2, SceneAsset, ISerializable)

SceneAsset::SceneAsset()
:	m_worldRenderSettings(new world::WorldRenderSettings())
{
}

Ref< Scene > SceneAsset::createScene(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	world::IEntityBuilder* entityBuilder,
	world::IEntityManager* entityManager
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

	return new Scene(
		controller,
		entityManager,
		rootEntity,
		m_worldRenderSettings,
		m_postProcessSettings
	);
}

void SceneAsset::setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings)
{
	m_worldRenderSettings = worldRenderSettings;
}

Ref< world::WorldRenderSettings > SceneAsset::getWorldRenderSettings() const
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

Ref< world::EntityInstance > SceneAsset::getInstance() const
{
	return m_instance;
}

void SceneAsset::setControllerData(ISceneControllerData* controllerData)
{
	m_controllerData = controllerData;
}

Ref< ISceneControllerData > SceneAsset::getControllerData() const
{
	return m_controllerData;
}

bool SceneAsset::serialize(ISerializer& s)
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
