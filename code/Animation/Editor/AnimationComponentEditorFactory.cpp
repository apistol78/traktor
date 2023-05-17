/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/SkeletonComponentData.h"
#include "Animation/Editor/AnimationComponentEditor.h"
#include "Animation/Editor/AnimationComponentEditorFactory.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimationComponentEditorFactory", AnimationComponentEditorFactory, scene::IComponentEditorFactory)

const TypeInfoSet AnimationComponentEditorFactory::getComponentDataTypes() const
{
	return makeTypeInfoSet< SkeletonComponentData >();
}


bool AnimationComponentEditorFactory::alwaysRebuild(const world::IEntityComponentData* componentData) const
{
	return true;
}

Ref< scene::IComponentEditor > AnimationComponentEditorFactory::createComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData) const
{
	return new AnimationComponentEditor(context, entityAdapter, componentData);
}

}
