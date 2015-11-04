#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Render/ITexture.h"
#include "Resource/Member.h"
#include "Scene/ISceneControllerData.h"
#include "Scene/Scene.h"
#include "Scene/Editor/SceneAsset.h"
#include "World/WorldRenderSettings.h"
#include "World/Editor/LayerEntityData.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.scene.SceneAsset", 7, SceneAsset, ISerializable)

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
	T_ASSERT (s.getVersion() >= 7);
	s >> MemberRef< world::WorldRenderSettings >(L"worldRenderSettings", m_worldRenderSettings);
	s >> MemberSmallMap< std::wstring, resource::Id< render::ITexture >, Member< std::wstring >, resource::Member< render::ITexture > >(L"imageProcessParams", m_imageProcessParams);
	s >> MemberRefArray< world::LayerEntityData >(L"layers", m_layers);
	s >> MemberRef< ISceneControllerData >(L"controllerData", m_controllerData);
}

	}
}
