/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Shape/Editor/Spline/SplineComponentData.h"
#include "Shape/Editor/Spline/SplineComponentEditor.h"
#include "Shape/Editor/Spline/SplineComponentEditorFactory.h"

namespace traktor::shape
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SplineComponentEditorFactory", SplineComponentEditorFactory, scene::IComponentEditorFactory)

const TypeInfoSet SplineComponentEditorFactory::getComponentDataTypes() const
{
	return makeTypeInfoSet< SplineComponentData >();
}

bool SplineComponentEditorFactory::alwaysRebuild(const world::IEntityComponentData* componentData) const
{
	return false;
}

Ref< scene::IComponentEditor > SplineComponentEditorFactory::createComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData) const
{
	return new SplineComponentEditor(context, entityAdapter, componentData);
}

}
