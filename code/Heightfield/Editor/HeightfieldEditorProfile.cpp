/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/HeightfieldFactory.h"
#include "Heightfield/Editor/HeightfieldEditorProfile.h"
#include "Scene/Editor/SceneEditorContext.h"

namespace traktor::hf
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.HeightfieldEditorProfile", 0, HeightfieldEditorProfile, scene::ISceneEditorProfile)

void HeightfieldEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
}

void HeightfieldEditorProfile::getGuideDrawIds(
	std::set< std::wstring >& outIds
) const
{
}

void HeightfieldEditorProfile::createEditorPlugins(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneEditorPlugin >& outEditorPlugins
) const
{
}

void HeightfieldEditorProfile::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< const resource::IResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(new HeightfieldFactory());
}

void HeightfieldEditorProfile::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< world::IEntityFactory >& outEntityFactories
) const
{
}

void HeightfieldEditorProfile::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	const TypeInfo& worldRendererType,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
}

void HeightfieldEditorProfile::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IWorldComponentEditorFactory >& outComponentEditorFactories) const
{
}

void HeightfieldEditorProfile::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
}

void HeightfieldEditorProfile::createComponentEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IComponentEditorFactory >& outComponentEditorFactories
) const
{
}

Ref< world::EntityData > HeightfieldEditorProfile::createEntityData(
	scene::SceneEditorContext* context,
	db::Instance* instance
) const
{
	return nullptr;
}

}
