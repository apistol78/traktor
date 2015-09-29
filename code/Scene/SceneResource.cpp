#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Render/ITexture.h"
#include "Render/ImageProcess/ImageProcessSettings.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Scene/ISceneControllerData.h"
#include "Scene/Scene.h"
#include "Scene/SceneResource.h"
#include "World/EntityData.h"
#include "World/EntitySchema.h"
#include "World/EntityBuilderWithSchema.h"
#include "World/IEntityFactory.h"
#include "World/WorldRenderSettings.h"

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
	world::IEntityBuilder* entityBuilder
) const
{
	resource::Proxy< render::ImageProcessSettings > imageProcessSettings[world::QuLast];
	SmallMap< render::handle_t, resource::Proxy< render::ITexture > > imageProcessParams;

	for (int32_t i = 0; i < world::QuLast; ++i)
	{
		if (m_imageProcessSettings[i].isNull())
			continue;

		if (!resourceManager->bind(m_imageProcessSettings[i], imageProcessSettings[i]))
			log::error << L"Unable to bind image processing settings " << i << Endl;
	}

	for (SmallMap< std::wstring, resource::Id< render::ITexture > >::const_iterator i = m_imageProcessParams.begin(); i != m_imageProcessParams.end(); ++i)
	{
		if (!resourceManager->bind(i->second, imageProcessParams[render::getParameterHandle(i->first)]))
			log::error << L"Unable to bind image processing parameter \"" << i->first << L"\"" << Endl;
	}

	Ref< world::IEntitySchema > entitySchema = new world::EntitySchema();
	std::map< const world::EntityData*, Ref< world::Entity > > entityProducts;

	Ref< world::EntityBuilderWithSchema > entityBuilderSchema = new world::EntityBuilderWithSchema(entityBuilder, entitySchema, entityProducts);
	Ref< world::Entity > rootEntity = entityBuilderSchema->create(m_entityData);

	Ref< ISceneController > controller;
	if (m_controllerData)
	{
		controller = m_controllerData->createController(entityProducts);
		if (!controller)
		{
			log::error << L"Failed to create scene; unable to instantiate scene controller" << Endl;
			return 0;
		}
	}

	T_ASSERT (entityBuilderSchema->getReferenceCount() == 1);

	return new Scene(
		controller,
		entitySchema,
		rootEntity,
		m_worldRenderSettings,
		imageProcessSettings,
		imageProcessParams
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

void SceneResource::setImageProcessSettings(world::Quality quality, const resource::Id< render::ImageProcessSettings >& imageProcessSettings)
{
	m_imageProcessSettings[int32_t(quality)] = imageProcessSettings;
}

const resource::Id< render::ImageProcessSettings >& SceneResource::getImageProcessSettings(world::Quality quality) const
{
	return m_imageProcessSettings[int32_t(quality)];
}

void SceneResource::setImageProcessParams(const SmallMap< std::wstring, resource::Id< render::ITexture > >& imageProcessParams)
{
	m_imageProcessParams = imageProcessParams;
}

const SmallMap< std::wstring, resource::Id< render::ITexture > >& SceneResource::getImageProcessParams() const
{
	return m_imageProcessParams;
}

void SceneResource::setEntityData(world::EntityData* entityData)
{
	m_entityData = entityData;
}

Ref< world::EntityData > SceneResource::getEntityData() const
{
	return m_entityData;
}

void SceneResource::setControllerData(ISceneControllerData* controllerData)
{
	m_controllerData = controllerData;
}

Ref< ISceneControllerData > SceneResource::getControllerData() const
{
	return m_controllerData;
}

void SceneResource::serialize(ISerializer& s)
{
	s >> MemberRef< world::WorldRenderSettings >(L"worldRenderSettings", m_worldRenderSettings);
	s >> MemberStaticArray< resource::Id< render::ImageProcessSettings >, sizeof_array(m_imageProcessSettings), resource::Member< render::ImageProcessSettings > >(L"postProcessSettings", m_imageProcessSettings);
	s >> MemberSmallMap< std::wstring, resource::Id< render::ITexture >, Member< std::wstring >, resource::Member< render::ITexture > >(L"postProcessParams", m_imageProcessParams);
	s >> MemberRef< world::EntityData >(L"entityData", m_entityData);
	s >> MemberRef< ISceneControllerData >(L"controllerData", m_controllerData);
}

	}
}
