/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Physics/Editor/PhysicsRenderer.h"
#include "Scene/Editor/IComponentEditor.h"

namespace traktor::scene
{

class EntityAdapter;
class SceneEditorContext;

}

namespace traktor::world
{

class IEntityComponentData;

}

namespace traktor::physics
{

class PhysicsComponentEditor : public scene::IComponentEditor
{
	T_RTTI_CLASS;

public:
	PhysicsComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData);

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const override final;

private:
	scene::SceneEditorContext* m_context;
	scene::EntityAdapter* m_entityAdapter;
	world::IEntityComponentData* m_componentData;
	PhysicsRenderer m_physicsRenderer;
};

}
