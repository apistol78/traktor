/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Cloth/ClothComponentData.h"
#include "Animation/Editor/Cloth/ClothEntityEditor.h"
#include "Animation/Editor/Cloth/ClothEntityEditorFactory.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.ClothEntityEditorFactory", ClothEntityEditorFactory, scene::IEntityEditorFactory)

const TypeInfoSet ClothEntityEditorFactory::getEntityDataTypes() const
{
	return makeTypeInfoSet< ClothComponentData >();
}

Ref< scene::IEntityEditor > ClothEntityEditorFactory::createEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter) const
{
	return new ClothEntityEditor(context, entityAdapter);
}

	}
}
