/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/PathEntity/PathComponentData.h"
#include "Animation/Editor/PathEntity/PathComponentEditor.h"
#include "Animation/Editor/PathEntity/PathComponentEditorFactory.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.PathComponentEditorFactory", PathComponentEditorFactory, scene::IComponentEditorFactory)

const TypeInfoSet PathComponentEditorFactory::getComponentDataTypes() const
{
	return makeTypeInfoSet< PathComponentData >();
}

bool PathComponentEditorFactory::alwaysRebuild(const world::IEntityComponentData* componentData) const
{
	return false;
}

Ref< scene::IComponentEditor > PathComponentEditorFactory::createComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData) const
{
	return new PathComponentEditor(context, entityAdapter, componentData);
}

}
