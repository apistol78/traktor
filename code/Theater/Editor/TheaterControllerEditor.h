/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Scene/Editor/ISceneControllerEditor.h"

namespace traktor
{
	namespace scene
	{

class PostFrameEvent;

	}

	namespace ui
	{

class CursorMoveEvent;
class EditList;
class EditListEditEvent;
class KeyMoveEvent;
class SelectionChangeEvent;
class ToolBar;
class ToolBarButtonClickEvent;
class SequencerControl;

	}

	namespace theater
	{

/*! Theater controller editor
 */
class TheaterControllerEditor : public scene::ISceneControllerEditor
{
	T_RTTI_CLASS;

public:
	virtual bool create(scene::SceneEditorContext* context, ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual void entityRemoved(scene::EntityAdapter* entityAdapter) override final;

	virtual void propertiesChanged() override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void update() override final;

	virtual void draw(render::PrimitiveRenderer* primitiveRenderer) override final;

private:
	Ref< ui::ToolBar > m_toolBarActs;
	Ref< ui::EditList > m_listActs;
	Ref< ui::ToolBar > m_toolBar;
	Ref< ui::SequencerControl > m_trackSequencer;
	Ref< scene::SceneEditorContext > m_context;
	//float m_timeOffset;

	void updateView();

	void captureEntities();

	void deleteSelectedKey();

	void setLookAtEntity();

	void easeVelocity();

	void gotoPreviousKey();

	void gotoNextKey();

	void splitAct();

	void timeScaleAct();

	void eventActSelected(ui::SelectionChangeEvent* event);

	void eventActEdit(ui::EditListEditEvent* event);

	void eventToolBarClick(ui::ToolBarButtonClickEvent* event);

	void eventSequencerCursorMove(ui::CursorMoveEvent* event);

	void eventSequencerKeyMove(ui::KeyMoveEvent* event);

	void eventContextPostFrame(scene::PostFrameEvent* event);
};

	}
}

