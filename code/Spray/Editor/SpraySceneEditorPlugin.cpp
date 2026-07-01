/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializable.h"
#include "Database/Instance.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Spray/Effect.h"
#include "Spray/EffectComponentData.h"
#include "Spray/EffectData.h"
#include "Spray/EffectEntityFactory.h"
#include "Spray/EffectFactory.h"
#include "Spray/EffectRenderer.h"
#include "Spray/Editor/SpraySceneEditorPlugin.h"
#include "Ui/Command.h"
#include "World/EntityData.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.SpraySceneEditorPlugin", 0, SpraySceneEditorPlugin, scene::ISceneEditorPlugin)

void SpraySceneEditorPlugin::getCommands(
	std::list< ui::Command >& outCommands
) const
{
}

void SpraySceneEditorPlugin::getGuideDrawIds(
	std::set< std::wstring >& outIds
) const
{
}

void SpraySceneEditorPlugin::createUIExtensions(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneEditorUIExtension >& outUIExtensions
) const
{
}

void SpraySceneEditorPlugin::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< const resource::IResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(new spray::EffectFactory(context->getRenderSystem(), nullptr));
}

void SpraySceneEditorPlugin::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(new spray::EffectEntityFactory());
}

void SpraySceneEditorPlugin::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	const TypeInfo& worldRendererType,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(new spray::EffectRenderer(context->getRenderSystem(), 50.0f, 100.0f));
}

void SpraySceneEditorPlugin::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IWorldComponentEditorFactory >& outComponentEditorFactories) const
{
}

void SpraySceneEditorPlugin::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
}

void SpraySceneEditorPlugin::createComponentEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IComponentEditorFactory >& outComponentEditorFactories
) const
{
}

Ref< world::EntityData > SpraySceneEditorPlugin::createEntityData(
	scene::SceneEditorContext* context,
	db::Instance* instance
) const
{
	const TypeInfo* primaryType = instance->getPrimaryType();
	if (!primaryType)
		return nullptr;

	if (!is_type_of< EffectData >(*primaryType))
		return nullptr;

	Ref< world::EntityData > entityData = new world::EntityData();
	entityData->setId(Guid::create());
	entityData->setName(instance->getName());
	entityData->setComponent(new EffectComponentData(
		resource::Id< Effect >(instance->getGuid())
	));
	return entityData;
}

}
