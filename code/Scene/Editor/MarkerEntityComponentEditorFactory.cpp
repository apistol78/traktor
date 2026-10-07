/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/MarkerEntityComponentEditorFactory.h"

#include "Scene/Editor/MarkerEntityComponentData.h"
#include "Scene/Editor/MarkerEntityComponentEditor.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.MarkerEntityComponentEditorFactory", MarkerEntityComponentEditorFactory, IComponentEditorFactory)

const TypeInfoSet MarkerEntityComponentEditorFactory::getComponentDataTypes() const
{
	return makeTypeInfoSet< MarkerEntityComponentData >();
}

bool MarkerEntityComponentEditorFactory::alwaysRebuild(const world::IEntityComponentData* componentData) const
{
	return false;
}

Ref< IComponentEditor > MarkerEntityComponentEditorFactory::createComponentEditor(SceneEditorContext* context, EntityAdapter* entityAdapter, world::IEntityComponentData* componentData) const
{
	return new MarkerEntityComponentEditor(context, entityAdapter, mandatory_non_null_type_cast< MarkerEntityComponentData* >(componentData));
}

}
