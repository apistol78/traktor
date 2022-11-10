/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/DefaultEntityEditor.h"
#include "Scene/Editor/DefaultEntityEditorFactory.h"
#include "World/EntityData.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.DefaultEntityEditorFactory", DefaultEntityEditorFactory, IEntityEditorFactory)

const TypeInfoSet DefaultEntityEditorFactory::getEntityDataTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< world::EntityData >());
	return typeSet;
}

Ref< IEntityEditor > DefaultEntityEditorFactory::createEntityEditor(SceneEditorContext* context, EntityAdapter* entityAdapter) const
{
	return new DefaultEntityEditor(context, entityAdapter);
}

	}
}
