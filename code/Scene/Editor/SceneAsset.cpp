#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/Member.h"
#include "Scene/ISceneControllerData.h"
#include "Scene/SceneResource.h"
#include "Scene/Editor/SceneAsset.h"
#include "World/WorldRenderSettings.h"
#include "World/Entity/EntityData.h"
#include "World/PostProcess/PostProcessSettings.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.scene.SceneAsset", 3, SceneAsset, editor::ITypedAsset)

SceneAsset::SceneAsset()
:	m_worldRenderSettings(new world::WorldRenderSettings())
{
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

void SceneAsset::setEntityData(world::EntityData* entityData)
{
	m_entityData = entityData;
}

Ref< world::EntityData > SceneAsset::getEntityData() const
{
	return m_entityData;
}

void SceneAsset::setControllerData(ISceneControllerData* controllerData)
{
	m_controllerData = controllerData;
}

Ref< ISceneControllerData > SceneAsset::getControllerData() const
{
	return m_controllerData;
}

const TypeInfo* SceneAsset::getOutputType() const
{
	return &type_of< SceneResource >();
}

bool SceneAsset::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 3);
	s >> MemberRef< world::WorldRenderSettings >(L"worldRenderSettings", m_worldRenderSettings);
	s >> resource::Member< world::PostProcessSettings >(L"postProcessSettings", m_postProcessSettings);
	s >> MemberRef< world::EntityData >(L"entityData", m_entityData);
	s >> MemberRef< ISceneControllerData >(L"controllerData", m_controllerData);
	return true;
}

	}
}
