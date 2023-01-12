/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Scene/Editor/IComponentEditorFactory.h"

namespace traktor
{
	namespace physics
	{

/*!
 */
class PhysicsComponentEditorFactory : public scene::IComponentEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getComponentDataTypes() const override final;

	virtual bool alwaysRebuild(const world::IEntityComponentData* componentData) const override final;

	virtual Ref< scene::IComponentEditor > createComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData) const override final;
};

	}
}

