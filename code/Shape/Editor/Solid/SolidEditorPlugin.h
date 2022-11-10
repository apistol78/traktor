/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Scene/Editor/ISceneEditorPlugin.h"

namespace traktor
{
	namespace scene
	{

class ModifierChangedEvent;

	}

	namespace shape
	{

/*! Solid scene editor plugin.
 * \ingroup Shape
 */
class SolidEditorPlugin : public scene::ISceneEditorPlugin
{
	T_RTTI_CLASS;

public:
	SolidEditorPlugin(scene::SceneEditorContext* context);

	virtual bool create(ui::Widget* parent, ui::ToolBar* toolBar) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

private:
	scene::SceneEditorContext* m_context;

	void eventModifierChanged(scene::ModifierChangedEvent* event);
};

	}
}
