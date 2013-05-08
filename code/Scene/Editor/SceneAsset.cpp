#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Resource/Member.h"
#include "Scene/ISceneControllerData.h"
#include "Scene/Scene.h"
#include "Scene/Editor/SceneAsset.h"
#include "World/WorldRenderSettings.h"
#include "World/Editor/LayerEntityData.h"
#include "World/PostProcess/PostProcessSettings.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.scene.SceneAsset", 4, SceneAsset, ISerializable)

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

void SceneAsset::setPostProcessSettings(const resource::Id< world::PostProcessSettings >& postProcessSettings)
{
	m_postProcessSettings = postProcessSettings;
}

const resource::Id< world::PostProcessSettings >& SceneAsset::getPostProcessSettings() const
{
	return m_postProcessSettings;
}

void SceneAsset::setLayers(const RefArray< world::LayerEntityData >& layers)
{
	m_layers = layers;
}

const RefArray< world::LayerEntityData >& SceneAsset::getLayers() const
{
	return m_layers;
}

void SceneAsset::setControllerData(ISceneControllerData* controllerData)
{
	m_controllerData = controllerData;
}

Ref< ISceneControllerData > SceneAsset::getControllerData() const
{
	return m_controllerData;
}

void SceneAsset::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 3);

	s >> MemberRef< world::WorldRenderSettings >(L"worldRenderSettings", m_worldRenderSettings);
	s >> resource::Member< world::PostProcessSettings >(L"postProcessSettings", m_postProcessSettings);

	if (s.getVersion() >= 4)
	{
		s >> MemberRefArray< world::LayerEntityData >(L"layers", m_layers);
	}
	else
	{
		Ref< world::EntityData > entityData;
		s >> MemberRef< world::EntityData >(L"entityData", entityData);

		Ref< world::LayerEntityData > layer = new world::LayerEntityData();
		if (world::GroupEntityData* groupEntityData = dynamic_type_cast< world::GroupEntityData* >(entityData))
		{
			layer->setName(groupEntityData->getName());
			layer->setEntityData(groupEntityData->getEntityData());
		}
		else
			layer->addEntityData(entityData);

		m_layers.push_back(layer);
	}
	
	s >> MemberRef< ISceneControllerData >(L"controllerData", m_controllerData);
}

	}
}
