/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializable.h"
#include "Theater/TheaterEntityFactory.h"
#include "Theater/Editor/TheaterSceneEditorPlugin.h"
#include "Theater/Editor/TheaterComponentEditorFactory.h"
#include "Ui/Command.h"

namespace traktor::theater
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.TheaterSceneEditorPlugin", 0, TheaterSceneEditorPlugin, scene::ISceneEditorPlugin)

void TheaterSceneEditorPlugin::getCommands(
	std::list< ui::Command >& outCommands
) const
{
	outCommands.push_back(ui::Command(L"Theater.CaptureEntities"));
	outCommands.push_back(ui::Command(L"Theater.DeleteSelectedKey"));
	outCommands.push_back(ui::Command(L"Theater.SetLookAtEntity"));
	outCommands.push_back(ui::Command(L"Theater.EaseVelocity"));
	outCommands.push_back(ui::Command(L"Theater.GotoPreviousKey"));
	outCommands.push_back(ui::Command(L"Theater.GotoNextKey"));
	outCommands.push_back(ui::Command(L"Theater.SplitAct"));
}

void TheaterSceneEditorPlugin::getGuideDrawIds(
	std::set< std::wstring >& outIds
) const
{
}

void TheaterSceneEditorPlugin::createUIExtensions(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneEditorUIExtension >& outUIExtensions
) const
{
}

void TheaterSceneEditorPlugin::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< const resource::IResourceFactory >& outResourceFactories
) const
{
}

void TheaterSceneEditorPlugin::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(new TheaterEntityFactory());
}

void TheaterSceneEditorPlugin::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	const TypeInfo& worldRendererType,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
}

void TheaterSceneEditorPlugin::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IWorldComponentEditorFactory >& outComponentEditorFactories) const
{
	outComponentEditorFactories.push_back(new TheaterComponentEditorFactory());
}

void TheaterSceneEditorPlugin::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
}

void TheaterSceneEditorPlugin::createComponentEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IComponentEditorFactory >& outComponentEditorFactories
) const
{
}

Ref< world::EntityData > TheaterSceneEditorPlugin::createEntityData(
	scene::SceneEditorContext* context,
	db::Instance* instance
) const
{
	return nullptr;
}

}
