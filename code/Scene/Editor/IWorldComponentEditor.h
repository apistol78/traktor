/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class PrimitiveRenderer;

}

namespace traktor::ui
{

class Container;
class Command;

}

namespace traktor::scene
{

class SceneEditorContext;
class EntityAdapter;

/*! World component editor interface.
 * \ingroup Scene
 */
class T_DLLCLASS IWorldComponentEditor : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool create(SceneEditorContext* context, ui::Container* parent) = 0;

	virtual void destroy() = 0;

	virtual void entityRemoved(EntityAdapter* entityAdapter) = 0;

	virtual void propertiesChanged() = 0;

	virtual bool handleCommand(const ui::Command& command) = 0;

	virtual void update() = 0;

	virtual void draw(render::PrimitiveRenderer* primitiveRenderer) = 0;
};

}
