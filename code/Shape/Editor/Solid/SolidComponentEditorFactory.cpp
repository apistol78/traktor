/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/EntityAdapter.h"
#include "Shape/Editor/Solid/SolidComponentData.h"
#include "Shape/Editor/Solid/SolidComponentEditor.h"
#include "Shape/Editor/Solid/SolidComponentEditorFactory.h"

namespace traktor::shape
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SolidComponentEditorFactory", SolidComponentEditorFactory, scene::IComponentEditorFactory)

const TypeInfoSet SolidComponentEditorFactory::getComponentDataTypes() const
{
	return makeTypeInfoSet< SolidComponentData >();
}

bool SolidComponentEditorFactory::alwaysRebuild(const world::IEntityComponentData* componentData) const
{
	return false;
}

Ref< scene::IComponentEditor > SolidComponentEditorFactory::createComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData) const
{
	return new SolidComponentEditor(context, entityAdapter, componentData);
}

}
