/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ai/NavMeshComponentData.h"
#include "Ai/Editor/NavMeshComponentEditor.h"
#include "Ai/Editor/NavMeshComponentEditorFactory.h"

namespace traktor::ai
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ai.NavMeshComponentEditorFactory", NavMeshComponentEditorFactory, scene::IComponentEditorFactory)

const TypeInfoSet NavMeshComponentEditorFactory::getComponentDataTypes() const
{
	return makeTypeInfoSet< NavMeshComponentData >();
}

bool NavMeshComponentEditorFactory::alwaysRebuild(const world::IEntityComponentData* componentData) const
{
	return false;
}

Ref< scene::IComponentEditor > NavMeshComponentEditorFactory::createComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData) const
{
	return new NavMeshComponentEditor(context, entityAdapter, componentData);
}

}
