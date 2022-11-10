/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Scene/Editor/DefaultEntityEditor.h"

namespace traktor
{
	namespace shape
	{

/*!
 * \ingroup Shape
 */
class SplineEntityEditor : public scene::DefaultEntityEditor
{
	T_RTTI_CLASS;

public:
	SplineEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter);

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const override final;
};

	}
}
