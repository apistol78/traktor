/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IEditor.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Shape/Editor/EditorProfile.h"
#include "Shape/Editor/EntityFactory.h"
#include "Shape/Editor/EntityRenderer.h"
#include "Shape/Editor/Solid/SolidEditorPlugin.h"
#include "Shape/Editor/Solid/SolidEntityEditorFactory.h"
#include "Shape/Editor/Spline/ControlPointComponentEditorFactory.h"
#include "Shape/Editor/Spline/SplineComponentEditorFactory.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.EditorProfile", 0, EditorProfile, scene::ISceneEditorProfile)

void EditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
	outCommands.push_back(ui::Command(L"Shape.Editor.EditPrimitive"));
	outCommands.push_back(ui::Command(L"Shape.Editor.BrowseMaterial"));
}

void EditorProfile::getGuideDrawIds(
	std::set< std::wstring >& outIds
) const
{
	outIds.insert(L"Shape.Solids");
}

void EditorProfile::createEditorPlugins(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneEditorPlugin >& outEditorPlugins
) const
{
	outEditorPlugins.push_back(new SolidEditorPlugin(context));
}

void EditorProfile::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< const resource::IResourceFactory >& outResourceFactories
) const
{
}

void EditorProfile::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< world::IEntityFactory >& outEntityFactories
) const
{
	const std::wstring assetPath = context->getEditor()->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath");
	const std::wstring modelCachePath = context->getEditor()->getSettings()->getProperty< std::wstring >(L"Pipeline.ModelCache.Path");

	outEntityFactories.push_back(new EntityFactory(
		assetPath,
		modelCachePath
	));
}

void EditorProfile::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	const TypeInfo& worldRendererType,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(new EntityRenderer());
}

void EditorProfile::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IWorldComponentEditorFactory >& outComponentEditorFactories) const
{
}

void EditorProfile::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
	outEntityEditorFactories.push_back(new SolidEntityEditorFactory());
}

void EditorProfile::createComponentEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IComponentEditorFactory >& outComponentEditorFactories
) const
{
	outComponentEditorFactories.push_back(new ControlPointComponentEditorFactory());
	outComponentEditorFactories.push_back(new SplineComponentEditorFactory());
}

Ref< world::EntityData > EditorProfile::createEntityData(
	scene::SceneEditorContext* context,
	db::Instance* instance
) const
{
	return nullptr;
}

	}
}
