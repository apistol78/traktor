/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/DefaultComponentEditor.h"
#include "Scene/Editor/DefaultComponentEditorFactory.h"
#include "World/IEntityComponentData.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.DefaultComponentEditorFactory", DefaultComponentEditorFactory, IComponentEditorFactory)

const TypeInfoSet DefaultComponentEditorFactory::getComponentDataTypes() const
{
	return makeTypeInfoSet< world::IEntityComponentData >();
}

bool DefaultComponentEditorFactory::alwaysRebuild(const world::IEntityComponentData* componentData) const
{
	return false;
}

Ref< IComponentEditor > DefaultComponentEditorFactory::createComponentEditor(SceneEditorContext* context, EntityAdapter* entityAdapter, world::IEntityComponentData* componentData) const
{
	return new DefaultComponentEditor(context, entityAdapter, componentData);
}

}
