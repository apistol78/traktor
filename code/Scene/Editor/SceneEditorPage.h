/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/CircularVector.h"
#include "Editor/IEditorPage.h"
#include "Ui/Events/AllEvents.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

}

namespace traktor::editor
{

class IDocument;
class IEditorPageSite;
class PropertiesView;

}

namespace traktor::ui
{

class Container;
class ContentChangeEvent;
class ContentChangingEvent;
class Font;
class IBitmap;
class Menu;
class StatusBar;
class Tab;
class ToolBar;
class ToolBarButton;
class ToolBarButtonClickEvent;
class GridColumnClickEvent;
class GridItemContentChangeEvent;
class GridRow;
class GridRowStateChangeEvent;
class GridView;

}

namespace traktor::world
{

class EntityData;

}

namespace traktor::scene
{

class CameraMovedEvent;
class EntityAdapter;
class EntityDependencyInvestigator;
class MeasurementEvent;
class PreModifyEvent;
class PostBuildEvent;
class PostFrameEvent;
class PostModifyEvent;
class SceneAsset;
class SceneEditorContext;
class ScenePreviewControl;

class T_DLLCLASS SceneEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	explicit SceneEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

private:
	editor::IEditor* m_editor = nullptr;
	editor::IEditorPageSite* m_site = nullptr;
	editor::IDocument* m_document = nullptr;
	Ref< SceneEditorContext > m_context;
	Ref< ui::Container > m_editPanel;
	Ref< ScenePreviewControl > m_editControl;
	Ref< ui::StatusBar > m_statusBar;
	Ref< ui::Container > m_entityPanel;
	Ref< ui::Tab > m_tabMisc;
	Ref< EntityDependencyInvestigator > m_entityDependencyPanel;
	Ref< ui::GridView > m_gridGuides;
	Ref< ui::GridView > m_gridMeasurements;
	Ref< ui::GridView > m_gridResources;
	Ref< ui::Menu > m_entityMenuDefault;
	Ref< ui::Menu > m_entityMenuGroup;
	Ref< ui::Menu > m_entityMenuExternal;
	Ref< ui::Container > m_controllerPanel;
	Ref< editor::PropertiesView > m_propertiesView;
	Ref< ui::ToolBar > m_entityToolBar;
	Ref< ui::ToolBarButton > m_buttonFilterEntity;
	Ref< ui::IBitmap > m_imageHidden;
	Ref< ui::IBitmap > m_imageVisible;
	Ref< ui::IBitmap > m_imageLocked;
	Ref< ui::IBitmap > m_imageUnlocked;
	Ref< ui::GridView > m_instanceGrid;
	Ref< ui::Font > m_instanceGridFontBold;
	Ref< ui::Font > m_instanceGridFontHuge;
	const TypeInfo* m_entityFilterType = nullptr;
	CircularVector< double, 256 > m_measurementVariance[64];

	bool createSceneAsset();

	void createControllerEditor();

	void updateScene();

	Ref< ui::GridRow > createInstanceGridRow(EntityAdapter* entityAdapter);

	void createInstanceGrid();

	void updateInstanceGridRow(ui::GridRow* row);

	void updateInstanceGrid();

	void updatePropertyObject();

	void updateStatusBar();

	bool addEntity(const TypeInfo* entityType);

	bool addComponent();

	bool createExternal();

	bool resolveExternal();

	bool moveToEntity();

	bool moveUp();

	bool moveDown();

	void placeOnGround();

	void eventEntityToolClick(ui::ToolBarButtonClickEvent* event);

	void eventGuideClick(ui::GridColumnClickEvent* event);

	void eventInstanceSelect(ui::SelectionChangeEvent* event);

	void eventInstanceExpand(ui::GridRowStateChangeEvent* event);

	void eventInstanceButtonDown(ui::MouseButtonDownEvent* event);

	void eventInstanceClick(ui::GridColumnClickEvent* event);

	void eventInstanceRename(ui::GridItemContentChangeEvent* event);

	void eventPropertiesChanging(ui::ContentChangingEvent* event);

	void eventPropertiesChanged(ui::ContentChangeEvent* event);

	void eventContextPostBuild(PostBuildEvent* event);

	void eventContextSelect(ui::SelectionChangeEvent* event);

	void eventContextPreModify(PreModifyEvent* event);

	void eventContextPostModify(PostModifyEvent* event);

	void eventContextCameraMoved(CameraMovedEvent* event);

	void eventContextPostFrame(PostFrameEvent* event);

	void eventContextMeasurement(MeasurementEvent* event);
};

}
