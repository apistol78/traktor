/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Editor/IEditorPage.h"
#include "Ui/Events/AllEvents.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IDocument;
class IEditor;
class IEditorPageSite;
class PropertiesView;

	}

	namespace ui
	{

class ContentChangeEvent;
class ContentChangingEvent;
class EdgeConnectEvent;
class EdgeDisconnectEvent;
class GraphControl;
class Menu;
class Node;
class NodeMovedEvent;
class SelectEvent;
class ToolBar;
class ToolBarButtonClickEvent;

	}

	namespace render
	{

class IgaPass;
class IgaTarget;
class ImageGraphAsset;
class Node;

class T_DLLEXPORT ImageGraphEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	ImageGraphEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< ui::ToolBar > m_toolBar;
	Ref< ImageGraphAsset > m_imageGraph;
	Ref< ui::GraphControl > m_editorGraph;
	Ref< editor::PropertiesView > m_propertiesView;
    Ref< ui::Menu > m_menuPopup;
	Ref< Node > m_propertiesNode;

	Ref< ui::Node > createEditorNode(Node* node) const;

	void createEditorGraph();

	void eventToolClick(ui::ToolBarButtonClickEvent* event);

    void eventButtonDown(ui::MouseButtonDownEvent* event);

	void eventSelect(ui::SelectEvent* event);

	void eventNodeMoved(ui::NodeMovedEvent* event);

	void eventEdgeConnect(ui::EdgeConnectEvent* event);

	void eventEdgeDisconnect(ui::EdgeDisconnectEvent* event);

	void eventPropertiesChanging(ui::ContentChangingEvent* event);

	void eventPropertiesChanged(ui::ContentChangeEvent* event);
};

	}
}

