#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Render/ITexture.h"
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
	SmallMap< render::handle_t, resource::Proxy< render::ITexture > > imageProcessParams;
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
		controller = m_controllerData->createController(entityProducts, false);
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
	s >> MemberSmallMap< std::wstring, resource::Id< render::ITexture >, Member< std::wstring >, resource::Member< render::ITexture > >(L"imageProcessParams", m_imageProcessParams);
	s >> MemberRef< world::EntityData >(L"entityData", m_entityData);
	s >> MemberRef< ISceneControllerData >(L"controllerData", m_controllerData);
}

	}
}
