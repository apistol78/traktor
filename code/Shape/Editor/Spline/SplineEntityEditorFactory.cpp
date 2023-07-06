/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Shape/Editor/Spline/SplineComponentData.h"
#include "Shape/Editor/Spline/SplineEntityEditor.h"
#include "Shape/Editor/Spline/SplineEntityEditorFactory.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SplineEntityEditorFactory", SplineEntityEditorFactory, scene::IEntityEditorFactory)

const TypeInfoSet SplineEntityEditorFactory::getEntityDataTypes() const
{
	return makeTypeInfoSet< SplineComponentData >();
}

Ref< scene::IEntityEditor > SplineEntityEditorFactory::createEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter) const 
{
	return new SplineEntityEditor(context, entityAdapter);
}

	}
}
