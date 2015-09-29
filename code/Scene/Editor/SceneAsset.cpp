#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Render/ITexture.h"
#include "Resource/Member.h"
#include "Render/ImageProcess/ImageProcessSettings.h"
#include "Scene/ISceneControllerData.h"
#include "Scene/Scene.h"
#include "Scene/Editor/SceneAsset.h"
#include "World/WorldRenderSettings.h"
#include "World/Editor/LayerEntityData.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const wchar_t* c_ImageProcessSettings_elementNames[] =
{
	L"disabled",
	L"low",
	L"medium",
	L"high",
	L"ultra"
};

		}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.scene.SceneAsset", 6, SceneAsset, ISerializable)

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

void SceneAsset::setImageProcessSettings(world::Quality quality, const resource::Id< render::ImageProcessSettings >& imageProcessSettings)
{
	m_imageProcessSettings[int32_t(quality)] = imageProcessSettings;
}

const resource::Id< render::ImageProcessSettings >& SceneAsset::getImageProcessSettings(world::Quality quality) const
{
	return m_imageProcessSettings[int32_t(quality)];
}

void SceneAsset::setImageProcessParams(const SmallMap< std::wstring, resource::Id< render::ITexture > >& imageProcessParams)
{
	m_imageProcessParams = imageProcessParams;
}

const SmallMap< std::wstring, resource::Id< render::ITexture > >& SceneAsset::getImageProcessParams() const
{
	return m_imageProcessParams;
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

	if (s.getVersion() >= 6)
		s >> MemberStaticArray< resource::Id< render::ImageProcessSettings >, sizeof_array(m_imageProcessSettings), resource::Member< render::ImageProcessSettings > >(L"imageProcessSettings", m_imageProcessSettings, c_ImageProcessSettings_elementNames);
	else
	{
		resource::Id< render::ImageProcessSettings > imageProcessSettings;
		s >> resource::Member< render::ImageProcessSettings >(L"imageProcessSettings", imageProcessSettings);
		for (int32_t i = 0; i < sizeof_array(m_imageProcessSettings); ++i)
			m_imageProcessSettings[i] = imageProcessSettings;
	}

	if (s.getVersion() >= 5)
		s >> MemberSmallMap< std::wstring, resource::Id< render::ITexture >, Member< std::wstring >, resource::Member< render::ITexture > >(L"imageProcessParams", m_imageProcessParams);

	if (s.getVersion() >= 4)
		s >> MemberRefArray< world::LayerEntityData >(L"layers", m_layers);
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
