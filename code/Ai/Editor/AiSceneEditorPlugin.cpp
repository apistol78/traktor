/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ai/NavMeshEntityFactory.h"
#include "Ai/NavMeshFactory.h"
#include "Ai/Editor/AiSceneEditorPlugin.h"
#include "Ai/Editor/NavMeshComponentEditorFactory.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/IEditor.h"
#include "Scene/Editor/SceneEditorContext.h"

namespace traktor::ai
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ai.AiSceneEditorPlugin", 0, AiSceneEditorPlugin, scene::ISceneEditorPlugin)

void AiSceneEditorPlugin::getCommands(
	std::list< ui::Command >& outCommands
) const
{
}

void AiSceneEditorPlugin::getGuideDrawIds(
	std::set< std::wstring >& outIds
) const
{
	outIds.insert(L"Ai.NavMesh");
}

void AiSceneEditorPlugin::createUIExtensions(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneEditorUIExtension >& outUIExtensions
) const
{
}

void AiSceneEditorPlugin::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< const resource::IResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(new NavMeshFactory());
}

void AiSceneEditorPlugin::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< world::IEntityFactory >& outEntityFactories
) const
{
	const bool build = context->getEditor()->getSettings()->getProperty< bool >(L"NavMeshPipeline.Build", true);
	outEntityFactories.push_back(new NavMeshEntityFactory(!build));
}

void AiSceneEditorPlugin::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	const TypeInfo& worldRendererType,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
}

void AiSceneEditorPlugin::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IWorldComponentEditorFactory >& outComponentEditorFactories) const
{
	outComponentEditorFactories.push_back(new NavMeshComponentEditorFactory());
}

void AiSceneEditorPlugin::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
}

void AiSceneEditorPlugin::createComponentEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IComponentEditorFactory >& outComponentEditorFactories
) const
{
}

Ref< world::EntityData > AiSceneEditorPlugin::createEntityData(
	scene::SceneEditorContext* context,
	db::Instance* instance
) const
{
	return nullptr;
}

}
