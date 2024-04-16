/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializable.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Terrain/EntityFactory.h"
#include "Terrain/EntityRenderer.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainComponent.h"
#include "Terrain/TerrainFactory.h"
#include "Terrain/TerrainSurfaceCache.h"
#include "Terrain/Editor/TerrainEditorPlugin.h"
#include "Terrain/Editor/TerrainEditorProfile.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.TerrainEditorProfile", 0, TerrainEditorProfile, scene::ISceneEditorProfile)

void TerrainEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
	outCommands.push_back(ui::Command(L"Terrain.Editor.EditTerrain"));
	outCommands.push_back(ui::Command(L"Terrain.Editor.AverageBrush"));
	outCommands.push_back(ui::Command(L"Terrain.Editor.ColorBrush"));
	outCommands.push_back(ui::Command(L"Terrain.Editor.CutBrush"));
	outCommands.push_back(ui::Command(L"Terrain.Editor.ElevateBrush"));
	outCommands.push_back(ui::Command(L"Terrain.Editor.FlattenBrush"));
	outCommands.push_back(ui::Command(L"Terrain.Editor.MaterialBrush"));
	outCommands.push_back(ui::Command(L"Terrain.Editor.NoiseBrush"));
	outCommands.push_back(ui::Command(L"Terrain.Editor.SmoothBrush"));
	outCommands.push_back(ui::Command(L"Terrain.Editor.SmoothFallOff"));
	outCommands.push_back(ui::Command(L"Terrain.Editor.SharpFallOff"));
}

void TerrainEditorProfile::getGuideDrawIds(
	std::set< std::wstring >& outIds
) const
{
	outIds.insert(L"Terrain.Heightfield");
}

void TerrainEditorProfile::createEditorPlugins(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneEditorPlugin >& outEditorPlugins
) const
{
	outEditorPlugins.push_back(new TerrainEditorPlugin(context));
}

void TerrainEditorProfile::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< const resource::IResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(new TerrainFactory());
}

void TerrainEditorProfile::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< const world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(new EntityFactory(
		context->getResourceManager(),
		context->getRenderSystem()
	));
}

void TerrainEditorProfile::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	const TypeInfo& worldRendererType,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(new EntityRenderer(100.0f, 8192, true, true));
}

void TerrainEditorProfile::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IWorldComponentEditorFactory >& outComponentEditorFactories) const
{
}

void TerrainEditorProfile::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
}

void TerrainEditorProfile::createComponentEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IComponentEditorFactory >& outComponentEditorFactories
) const
{
}

Ref< world::EntityData > TerrainEditorProfile::createEntityData(
	scene::SceneEditorContext* context,
	db::Instance* instance
) const
{
	return nullptr;
}

	}
}
