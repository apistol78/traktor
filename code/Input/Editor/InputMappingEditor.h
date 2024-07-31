/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include "Editor/IEditorPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

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
class EdgeConnectEvent;
class EdgeDisconnectEvent;
class EditList;
class EditListEditEvent;
class GraphControl;
class MouseButtonDownEvent;
class Menu;
class NodeActivateEvent;
class NodeMovedEvent;
class SelectEvent;
class SelectionChangeEvent;
class ToolBar;
class ToolBarButtonClickEvent;

}

namespace traktor::input
{

class InputMappingAsset;
class InputNodeTraits;

class T_DLLCLASS InputMappingEditor : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	InputMappingEditor(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	std::map< const TypeInfo*, Ref< const InputNodeTraits > > m_traits;
	Ref< InputMappingAsset > m_mappingAsset;
	Ref< ui::ToolBar > m_toolBarGraph;
	Ref< ui::EditList > m_listValueSources;
	Ref< ui::GraphControl > m_graph;
	Ref< editor::PropertiesView > m_propertiesView;
	Ref< ui::Menu > m_menuPopup;

	void updateGraphView();

	void updatePropertiesView();

	void eventToolBarGraphClick(ui::ToolBarButtonClickEvent* event);

	void eventButtonDown(ui::MouseButtonDownEvent* event);

	void eventListValueSourceSelect(ui::SelectionChangeEvent* event);

	void eventListValueEdit(ui::EditListEditEvent* event);

	void eventNodeSelect(ui::SelectEvent* event);

	void eventNodeMoved(ui::NodeMovedEvent* event);

	void eventNodeActivated(ui::NodeActivateEvent* event);

	void eventEdgeConnected(ui::EdgeConnectEvent* event);

	void eventEdgeDisconnected(ui::EdgeDisconnectEvent* event);

	void eventPropertiesChanging(ui::ContentChangingEvent* event);

	void eventPropertiesChanged(ui::ContentChangeEvent* event);
};

}
