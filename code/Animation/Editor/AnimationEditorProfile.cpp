/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/AnimationEntityFactory.h"
#include "Animation/AnimatedMeshComponentRenderer.h"
#include "Animation/AnimationResourceFactory.h"
#include "Animation/Cloth/ClothRenderer.h"
#include "Animation/Editor/AnimationEditorProfile.h"
#include "Animation/Editor/AnimationComponentEditorFactory.h"
#include "Animation/Editor/Cloth/ClothEntityEditorFactory.h"
#include "Core/Serialization/ISerializable.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.AnimationEditorProfile", 0, AnimationEditorProfile, scene::ISceneEditorProfile)

void AnimationEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
}

void AnimationEditorProfile::getGuideDrawIds(
	std::set< std::wstring >& outIds
) const
{
	outIds.insert(L"Animation.Cloth");
	outIds.insert(L"Animation.Path");
	outIds.insert(L"Animation.Skeleton.Bind");
	outIds.insert(L"Animation.Skeleton.Pose");
	outIds.insert(L"Animation.Controller");
}

void AnimationEditorProfile::createEditorPlugins(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneEditorPlugin >& outEditorPlugins
) const
{
}

void AnimationEditorProfile::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< const resource::IResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(new AnimationResourceFactory());
}

void AnimationEditorProfile::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(new AnimationEntityFactory());
}

void AnimationEditorProfile::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	const TypeInfo& worldRendererType,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(new AnimatedMeshComponentRenderer());
	outEntityRenderers.push_back(new ClothRenderer());
}

void AnimationEditorProfile::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IWorldComponentEditorFactory >& outComponentEditorFactories) const
{
}

void AnimationEditorProfile::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
	outEntityEditorFactories.push_back(new ClothEntityEditorFactory());
}

void AnimationEditorProfile::createComponentEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IComponentEditorFactory >& outComponentEditorFactories
) const
{
	outComponentEditorFactories.push_back(new AnimationComponentEditorFactory());
}

Ref< world::EntityData > AnimationEditorProfile::createEntityData(
	scene::SceneEditorContext* context,
	db::Instance* instance
) const
{
	return nullptr;
}

}
