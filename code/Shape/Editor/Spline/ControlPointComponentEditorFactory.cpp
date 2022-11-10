/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Shape/Editor/Spline/ControlPointComponentData.h"
#include "Shape/Editor/Spline/ControlPointComponentEditor.h"
#include "Shape/Editor/Spline/ControlPointComponentEditorFactory.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.ControlPointComponentEditorFactory", ControlPointComponentEditorFactory, scene::IComponentEditorFactory)

const TypeInfoSet ControlPointComponentEditorFactory::getComponentDataTypes() const
{
	return makeTypeInfoSet< ControlPointComponentData >();
}

Ref< scene::IComponentEditor > ControlPointComponentEditorFactory::createComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData) const
{
	return new ControlPointComponentEditor(context, entityAdapter, componentData);
}

	}
}
