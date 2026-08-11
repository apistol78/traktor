/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Scene/Editor/IComponentPanelEditor.h"
#include "Ui/EventSubject.h"

namespace traktor::scene
{

class PostFrameEvent;

}

namespace traktor::ui
{

class Container;
class CursorMoveEvent;
class KeyMoveEvent;
class SelectionChangeEvent;
class ToolBar;
class ToolBarButtonClickEvent;
class ToolBarDropDown;
class SequencerControl;

}

namespace traktor::theater
{

class ActData;
class TheaterEditTarget;

/*! Theater editor.
 *
 * Edit acts of any theater component of the scene; either the world
 * component, animating any entity of the scene, or an entity component
 * animating child entities of a single entity.
 */
class TheaterEditor : public scene::IComponentPanelEditor
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getTitle() const override final;

	virtual bool create(scene::SceneEditorContext* context, ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual void entityRemoved(scene::EntityAdapter* entityAdapter) override final;

	virtual void propertiesChanged() override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void update() override final;

	virtual void draw(render::PrimitiveRenderer* primitiveRenderer) override final;

private:
	Ref< ui::Container > m_container;
	Ref< ui::ToolBarDropDown > m_dropDownTargets;
	Ref< ui::ToolBarDropDown > m_dropDownActs;
	Ref< ui::ToolBar > m_toolBar;
	Ref< ui::SequencerControl > m_trackSequencer;
	Ref< scene::SceneEditorContext > m_context;
	Ref< ui::EventSubject::IEventHandler > m_eventHandlerPostFrame;
	RefArray< TheaterEditTarget > m_targets;
	uint32_t m_buildCount = 0;
	bool m_refreshProperties = false;

	/*! Collect all theater components of the scene. */
	void updateTargets();

	/*! Get selected target, null if none selected. */
	TheaterEditTarget* getTarget() const;

	/*! Get selected act, null if no act ("<None>") is selected. */
	ActData* getAct() const;

	/*! Re-instantiate the selected target after its acts have been modified. */
	void rebuildTarget();

	void updateView();

	void captureEntities();

	void deleteSelectedKey();

	void setLookAtEntity();

	void easeVelocity();

	void gotoPreviousKey();

	void gotoNextKey();

	void splitAct();

	void timeScaleAct();

	void eventToolBarClick(ui::ToolBarButtonClickEvent* event);

	void eventSequencerCursorMove(ui::CursorMoveEvent* event);

	void eventSequencerKeyMove(ui::KeyMoveEvent* event);

	void eventContextPostFrame(scene::PostFrameEvent* event);
};

}
