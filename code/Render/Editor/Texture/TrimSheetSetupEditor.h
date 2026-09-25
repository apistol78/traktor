/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Editor/IEditorPage.h"
#include "Render/Editor/Texture/TrimSheetSlab.h"
#include "Render/Editor/Texture/TrimSheetRegion.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::drawing
{

class Image;

}

namespace traktor::editor
{

class IDocument;
class IEditor;
class IEditorPageSite;
class PropertiesView;

}

namespace traktor::ui
{

class ContentChangeEvent;
class ContentChangingEvent;
class MouseButtonUpEvent;
class MouseMoveEvent;
class SelectionChangeEvent;
class StatusBar;
class ToolBar;
class ToolBarButton;
class ToolBarButtonClickEvent;
class ToolBarDropDown;
class TreeView;

}

namespace traktor::render
{

class TrimSheetComposer;
class TrimSheetControl;
class TrimSheetSetupAsset;

/*! Trim sheet setup editor.
 * \ingroup Render
 */
class T_DLLCLASS TrimSheetSetupEditor : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	explicit TrimSheetSetupEditor(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< TrimSheetSetupAsset > m_asset;
	Ref< TrimSheetComposer > m_composer;
	Ref< drawing::Image > m_sheet;
	std::wstring m_assetPath;
	TrimSheetLayer m_layer = TrimSheetLayer::Albedo;
	int32_t m_selectedSlab = -1;
	int32_t m_selectedRegion = -1;
	bool m_sheetDirty = false;
	bool m_propertiesDirty = false;

	Ref< ui::ToolBar > m_toolBar;
	Ref< ui::ToolBarDropDown > m_toolLayer;
	Ref< ui::ToolBarButton > m_toolToggleGuides;
	Ref< ui::ToolBarButton > m_toolToggleNames;
	Ref< ui::TreeView > m_treeStructure;
	Ref< TrimSheetControl > m_control;
	Ref< ui::StatusBar > m_statusBar;
	Ref< editor::PropertiesView > m_propertiesView;

	/*! Rebuild structure tree. */
	void updateTree();

	/*! Compose current layer and show it. */
	void updateSheet();

	/*! Propagate selection to control, tree and properties. */
	void updateSelection(bool updateProperties);

	/*! Show bounds of selected region's image in current layer. */
	void updateImageBounds();

	void updateStatus();

	/*! Ensure selection is valid, after undo or structural changes. */
	void validateSelection();

	/*! Make source image paths relative to asset path, if possible. */
	bool makeRelativePaths(TrimSheetRegion* region) const;

	void addSlab(TrimSheetSlab::Orientation orientation);

	void splitRegion();

	void removeSelected();

	void moveSelected(int32_t direction);

	void rotateImage();

	void exportLayer(bool guides);

	void exportAllLayers();

	void eventToolClick(ui::ToolBarButtonClickEvent* event);

	void eventTreeSelect(ui::SelectionChangeEvent* event);

	void eventControlSelect(ui::SelectionChangeEvent* event);

	void eventControlChanging(ui::ContentChangingEvent* event);

	void eventControlChange(ui::ContentChangeEvent* event);

	void eventControlMouseUp(ui::MouseButtonUpEvent* event);

	void eventControlMouseMove(ui::MouseMoveEvent* event);

	void eventPropertiesChanging(ui::ContentChangingEvent* event);

	void eventPropertiesChanged(ui::ContentChangeEvent* event);
};

}
