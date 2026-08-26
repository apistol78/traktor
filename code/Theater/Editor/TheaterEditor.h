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
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Transform.h"
#include "Scene/Editor/IComponentPanelEditor.h"
#include "Ui/EventSubject.h"

namespace traktor::scene
{

class PostFrameEvent;
class SceneSelectionChangeEvent;

}

namespace traktor::world
{

class EntityData;
class IEntityComponentData;

}

namespace traktor::ui
{

class Container;
class CursorMoveEvent;
class Key;
class KeySelectEvent;
class KeyMoveEvent;
class SelectionChangeEvent;
class ToolBar;
class ToolBarButton;
class ToolBarButtonClickEvent;
class ToolBarDropDown;
class ToolBarItem;
class SequencerControl;

}

namespace traktor::theater
{

class ActData;
class PropertyPath;
class TheaterEditTarget;
class TrackData;

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
	/*! State of an entity as it was before the theater mode was entered. */
	struct EntityState
	{
		Ref< world::EntityData > entityData;
		Transform transform;
		RefArray< world::IEntityComponentData > components;
	};

	Ref< ui::Container > m_container;
	Ref< ui::ToolBarButton > m_toolToggleMode;
	RefArray< ui::ToolBarItem > m_toolItemsInMode;
	Ref< ui::ToolBarDropDown > m_dropDownTargets;
	Ref< ui::ToolBarDropDown > m_dropDownActs;
	Ref< ui::ToolBar > m_toolBar;
	Ref< ui::SequencerControl > m_trackSequencer;
	Ref< scene::SceneEditorContext > m_context;
	Ref< ui::EventSubject::IEventHandler > m_eventHandlerPostFrame;
	Ref< ui::EventSubject::IEventHandler > m_eventHandlerSelect;
	RefArray< TheaterEditTarget > m_targets;
	AlignedVector< EntityState > m_originalState;
	AlignedVector< EntityState > m_sceneState;
	uint32_t m_buildCount = 0;
	double m_invalidatedTime = -1.0;
	bool m_mode = false;
	bool m_keysMoved = false;
	bool m_refreshProperties = false;

	/*! Check if a key of the sequencer is being dragged. */
	bool isTrackingKey() const;

	/*! Get the selected key of the sequencer; null if no key is selected. */
	ui::Key* findSelectedKey() const;

	/*! Select the key of a path at a given time, if the sequencer show one. */
	void selectKey(const PropertyPath* propertyPath, float time);

	/*! Put the state of the components, as it was captured, back into the scene. */
	void applyCapturedComponentData(const ui::Key* key);

	/*! Enter, or leave, the theater mode.
	 *
	 * The state of the scene is remembered as the mode is entered and restored
	 * as it is left, so an act can be authored without modifying the scene.
	 */
	void setMode(bool mode);

	/*! Remember the state of the entities animated by the theater components. */
	void rememberSceneState();

	/*! Remember the unposed state of an entity, unless it already is known. */
	void rememberOriginalState(const scene::EntityAdapter* entityAdapter);

	/*! Remember an entity as being animated by the theater, unless it already is. */
	void rememberEntityState(const scene::EntityAdapter* entityAdapter);

	/*! Read the current state of an entity; false if it has no data of its own. */
	bool readEntityState(const scene::EntityAdapter* entityAdapter, EntityState& outState) const;

	/*! Restore the state of every entity animated by the theater. */
	void restoreSceneState();

	/*! Collect all theater components of the scene. */
	void updateTargets();

	/*! Get selected target, null if none selected. */
	TheaterEditTarget* getTarget() const;

	/*! Get selected act, null if no act ("<None>") is selected. */
	ActData* getAct() const;

	/*! Re-instantiate the selected target after its acts have been modified.
	 *
	 * \param refreshProperties Refresh the property view as well; refreshing it
	 *                          rebuilds the entities, which undo the pose.
	 */
	void rebuildTarget(bool refreshProperties = true);

	/*! Invalidate cached components which the performance is driving, as they no longer reflect their data. */
	void invalidateAnimatedComponents();

	void updateView();

	/*! Capture interpolateable properties, the transform included, of the selected entities. */
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

	void eventSequencerKeySelect(ui::KeySelectEvent* event);

	void eventContextPostFrame(scene::PostFrameEvent* event);

	void eventContextSelect(scene::SceneSelectionChangeEvent* event);
};

}
