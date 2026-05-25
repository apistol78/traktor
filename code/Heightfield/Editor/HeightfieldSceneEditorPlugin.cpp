/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/HeightfieldFactory.h"
#include "Heightfield/Editor/HeightfieldSceneEditorPlugin.h"
#include "Scene/Editor/SceneEditorContext.h"

namespace traktor::hf
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.HeightfieldSceneEditorPlugin", 0, HeightfieldSceneEditorPlugin, scene::ISceneEditorPlugin)

void HeightfieldSceneEditorPlugin::getCommands(
	std::list< ui::Command >& outCommands
) const
{
}

void HeightfieldSceneEditorPlugin::getGuideDrawIds(
	std::set< std::wstring >& outIds
) const
{
}

void HeightfieldSceneEditorPlugin::createUIExtensions(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneEditorUIExtension >& outUIExtensions
) const
{
}

void HeightfieldSceneEditorPlugin::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< const resource::IResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(new HeightfieldFactory());
}

void HeightfieldSceneEditorPlugin::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< world::IEntityFactory >& outEntityFactories
) const
{
}

void HeightfieldSceneEditorPlugin::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	const TypeInfo& worldRendererType,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
}

void HeightfieldSceneEditorPlugin::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IWorldComponentEditorFactory >& outComponentEditorFactories) const
{
}

void HeightfieldSceneEditorPlugin::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
}

void HeightfieldSceneEditorPlugin::createComponentEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IComponentEditorFactory >& outComponentEditorFactories
) const
{
}

Ref< world::EntityData > HeightfieldSceneEditorPlugin::createEntityData(
	scene::SceneEditorContext* context,
	db::Instance* instance
) const
{
	return nullptr;
}

}
