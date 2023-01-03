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
#include "Animation/Editor/AnimatedMeshComponentEditorFactory.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimatedMeshComponentEditorFactory", AnimatedMeshComponentEditorFactory, scene::IComponentEditorFactory)

const TypeInfoSet AnimatedMeshComponentEditorFactory::getComponentDataTypes() const
{
	return makeTypeInfoSet< SkeletonComponentData >();
}

Ref< scene::IComponentEditor > AnimatedMeshComponentEditorFactory::createComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData) const
{
	return new AnimationComponentEditor(context, entityAdapter, componentData);
}

	}
}
