/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Editor/IEditorPage.h"
#include "Ui/Point.h"

#include <map>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
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

class ButtonClickEvent;
class Container;
class ContentChangeEvent;
class EdgeConnectEvent;
class EdgeDisconnectEvent;
class GraphControl;
class Menu;
class MouseButtonDownEvent;
class Node;
class NodeMovedEvent;
class SelectionChangeEvent;
class ToolBar;
class ToolBarButtonClickEvent;

}

namespace traktor::animation
{

class AnimationPreviewControl;
class StateGraph;
class StateNode;
class StateTransition;

/*!
 * \ingroup Animation
 */
class T_DLLEXPORT StateGraphEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	explicit StateGraphEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< StateGraph > m_stateGraph;
	Ref< ui::ToolBar > m_toolBarGraph;
	Ref< ui::GraphControl > m_editorGraph;
	Ref< ui::Menu > m_menuPopup;
	Ref< editor::PropertiesView > m_propertiesView;
	Ref< ui::Container > m_containerPreview;
	Ref< ui::ToolBar > m_toolBarPreview;
	Ref< AnimationPreviewControl > m_previewControl;
	Ref< ui::Container > m_previewConditions;

	void createEditorNodes(const RefArray< StateNode >& states, const RefArray< StateTransition >& transitions);

	Ref< ui::Node > createEditorNode(StateNode* state);

	void createState(const ui::Point& at, const TypeInfo& stateType);

	void updateGraph();

	void updatePreview(const StateGraph* stateGraph);

	void updatePreviewConditions();

	void eventToolBarGraphClick(ui::ToolBarButtonClickEvent* event);

	void eventToolBarPreviewClick(ui::ToolBarButtonClickEvent* event);

	void eventButtonDown(ui::MouseButtonDownEvent* event);

	void eventSelect(ui::SelectionChangeEvent* event);

	void eventNodeMoved(ui::NodeMovedEvent* event);

	void eventEdgeConnect(ui::EdgeConnectEvent* event);

	void eventEdgeDisconnect(ui::EdgeDisconnectEvent* event);

	void eventPropertiesChanged(ui::ContentChangeEvent* event);

	void eventPreviewConditionClick(ui::ButtonClickEvent* event);
};

}
