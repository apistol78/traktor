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
	world::IEntityBuilder* entityBuilder
) const
{
	resource::Proxy< world::PostProcessSettings > postProcessSettings[world::QuLast];
	SmallMap< render::handle_t, resource::Proxy< render::ITexture > > postProcessParams;

	for (int32_t i = 0; i < world::QuLast; ++i)
	{
		if (m_postProcessSettings[i].isNull())
			continue;

		if (!resourceManager->bind(m_postProcessSettings[i], postProcessSettings[i]))
			log::error << L"Unable to bind post processing settings " << i << Endl;
	}

	for (SmallMap< std::wstring, resource::Id< render::ITexture > >::const_iterator i = m_postProcessParams.begin(); i != m_postProcessParams.end(); ++i)
	{
		if (!resourceManager->bind(i->second, postProcessParams[render::getParameterHandle(i->first)]))
			log::error << L"Unable to bind post processing parameter \"" << i->first << L"\"" << Endl;
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
		postProcessSettings,
		postProcessParams
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

void SceneResource::setPostProcessSettings(world::Quality quality, const resource::Id< world::PostProcessSettings >& postProcessSettings)
{
	m_postProcessSettings[int32_t(quality)] = postProcessSettings;
}

const resource::Id< world::PostProcessSettings >& SceneResource::getPostProcessSettings(world::Quality quality) const
{
	return m_postProcessSettings[int32_t(quality)];
}

void SceneResource::setPostProcessParams(const SmallMap< std::wstring, resource::Id< render::ITexture > >& postProcessParams)
{
	m_postProcessParams = postProcessParams;
}

const SmallMap< std::wstring, resource::Id< render::ITexture > >& SceneResource::getPostProcessParams() const
{
	return m_postProcessParams;
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
	s >> MemberStaticArray< resource::Id< world::PostProcessSettings >, sizeof_array(m_postProcessSettings), resource::Member< world::PostProcessSettings > >(L"postProcessSettings", m_postProcessSettings);
	s >> MemberSmallMap< std::wstring, resource::Id< render::ITexture >, Member< std::wstring >, resource::Member< render::ITexture > >(L"postProcessParams", m_postProcessParams);
	s >> MemberRef< world::EntityData >(L"entityData", m_entityData);
	s >> MemberRef< ISceneControllerData >(L"controllerData", m_controllerData);
}

	}
}
