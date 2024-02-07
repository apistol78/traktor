/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializable.h"
#include "Database/Instance.h"
#include "Scene/Editor/DefaultEditorProfile.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"
#include "World/Deferred/WorldRendererDeferred.h"
#include "World/Entity/ExternalEntityData.h"

// Resource factories
#include "Render/Image2/ImageGraphFactory.h"
#include "Render/Resource/AliasTextureFactory.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/TextureFactory.h"
#include "Script/ScriptFactory.h"
#include "Sound/AudioResourceFactory.h"
#include "Video/VideoFactory.h"
#include "Weather/Clouds/CloudMaskFactory.h"
#include "World/WorldResourceFactory.h"

// Entity factories
#include "Weather/WeatherFactory.h"
#include "World/Entity/WorldEntityFactory.h"
#include "World/Editor/WorldEditorEntityFactory.h"

// Entity renderers
#include "World/Entity/DecalRenderer.h"
#include "World/Entity/ProbeRenderer.h"
#include "World/Entity/VolumetricFogRenderer.h"
#include "Weather/Clouds/CloudRenderer.h"
#include "Weather/Precipitation/PrecipitationRenderer.h"
#include "Weather/Sky/SkyRenderer.h"

// Entity editor factories
#include "Scene/Editor/DefaultComponentEditorFactory.h"
#include "Scene/Editor/DefaultEntityEditorFactory.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.DefaultEditorProfile", 0, DefaultEditorProfile, ISceneEditorProfile)

void DefaultEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
}

void DefaultEditorProfile::getGuideDrawIds(
	std::set< std::wstring >& outIds
) const
{
	outIds.insert(L"Entity.Light");
	outIds.insert(L"Entity.BoundingBox");
	outIds.insert(L"Entity.Volumes");
}

void DefaultEditorProfile::createEditorPlugins(
	SceneEditorContext* context,
	RefArray< ISceneEditorPlugin >& outEditorPlugins
) const
{
}

void DefaultEditorProfile::createResourceFactories(
	SceneEditorContext* context,
	RefArray< const resource::IResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(new render::AliasTextureFactory());
	outResourceFactories.push_back(new render::ImageGraphFactory(context->getRenderSystem()));
	outResourceFactories.push_back(new render::ShaderFactory(context->getRenderSystem()));
	outResourceFactories.push_back(new render::TextureFactory(context->getRenderSystem(), 0));
	outResourceFactories.push_back(new sound::AudioResourceFactory());
	outResourceFactories.push_back(new video::VideoFactory(context->getRenderSystem()));
	outResourceFactories.push_back(new weather::CloudMaskFactory());
	outResourceFactories.push_back(new world::WorldResourceFactory(context->getRenderSystem(), nullptr));
	outResourceFactories.push_back(new script::ScriptFactory(context->getScriptContext()));
}

void DefaultEditorProfile::createEntityFactories(
	SceneEditorContext* context,
	RefArray< const world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(new world::WorldEntityFactory(context->getResourceManager(), context->getRenderSystem(), true));
	outEntityFactories.push_back(new world::WorldEditorEntityFactory());
	outEntityFactories.push_back(new weather::WeatherFactory(context->getResourceManager(), context->getRenderSystem()));
}

void DefaultEditorProfile::createEntityRenderers(
	SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	const TypeInfo& worldRendererType,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(new world::DecalRenderer(context->getRenderSystem()));
	outEntityRenderers.push_back(new world::ProbeRenderer(context->getResourceManager(), context->getRenderSystem(), worldRendererType));
	outEntityRenderers.push_back(new world::VolumetricFogRenderer());
	outEntityRenderers.push_back(new weather::CloudRenderer(primitiveRenderer));
	outEntityRenderers.push_back(new weather::PrecipitationRenderer());
	outEntityRenderers.push_back(new weather::SkyRenderer());
}

void DefaultEditorProfile::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::ISceneControllerEditorFactory >& outControllerEditorFactories
) const
{
}

void DefaultEditorProfile::createEntityEditorFactories(
	SceneEditorContext* context,
	RefArray< const IEntityEditorFactory >& outEntityEditorFactories
) const
{
	outEntityEditorFactories.push_back(new DefaultEntityEditorFactory());
}

void DefaultEditorProfile::createComponentEditorFactories(
	SceneEditorContext* context,
	RefArray< const IComponentEditorFactory >& outComponentEditorFactories
) const
{
	outComponentEditorFactories.push_back(new DefaultComponentEditorFactory());
}

Ref< world::EntityData > DefaultEditorProfile::createEntityData(
	SceneEditorContext* context,
	db::Instance* instance
) const
{
	Ref< const world::EntityData > externalEntityData = instance->getObject< world::EntityData >();
	if (!externalEntityData)
		return nullptr;

	// Create external reference to entity data.
	Ref< world::EntityData > entityData = new world::ExternalEntityData(resource::Id< world::EntityData >(instance->getGuid()));
	entityData->setId(Guid::create());
	entityData->setName(externalEntityData->getName());
	return entityData;
}

}
