/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/EntityAdapter.h"
#include "Shape/Editor/Solid/SolidEntityData.h"
#include "Shape/Editor/Solid/SolidEntityEditor.h"
#include "Shape/Editor/Solid/SolidEntityEditorFactory.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SolidEntityEditorFactory", SolidEntityEditorFactory, scene::IEntityEditorFactory)

const TypeInfoSet SolidEntityEditorFactory::getEntityDataTypes() const
{
	return makeTypeInfoSet< SolidEntityData >();
}

Ref< scene::IEntityEditor > SolidEntityEditorFactory::createEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter) const
{
	return new SolidEntityEditor(context, entityAdapter);
}

	}
}
