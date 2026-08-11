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

#include <string>

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

/*! Component panel editor interface.
 *
 * An editor of a component, either a world component or an entity component,
 * which need more elaborate editing than the property view can offer. Such
 * an editor is presented in its own page of the component panel, and can
 * also draw guides in the 3d views.
 *
 * \ingroup Scene
 */
class T_DLLCLASS IComponentPanelEditor : public Object
{
	T_RTTI_CLASS;

public:
	/*! Title of editor as presented to the user. */
	virtual std::wstring getTitle() const = 0;

	/*! Create editor UI; an editor which doesn't create any UI will not get a page in the component panel. */
	virtual bool create(SceneEditorContext* context, ui::Container* parent) = 0;

	virtual void destroy() = 0;

	virtual void entityRemoved(EntityAdapter* entityAdapter) = 0;

	virtual void propertiesChanged() = 0;

	virtual bool handleCommand(const ui::Command& command) = 0;

	virtual void update() = 0;

	virtual void draw(render::PrimitiveRenderer* primitiveRenderer) = 0;
};

}
