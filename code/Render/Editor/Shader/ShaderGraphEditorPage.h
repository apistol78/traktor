/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ShaderGraphEditorPage_H
#define traktor_render_ShaderGraphEditorPage_H

#include <map>
#include "Core/RefArray.h"
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

	}

	namespace ui
	{

class Container;
class Point;
class PopupMenu;

		namespace custom
		{

class EdgeConnectEvent;
class EdgeDisconnectEvent;
class GraphControl;
class Node;
class NodeActivateEvent;
class NodeMovedEvent;
class SelectEvent;
class ToolBar;
class ToolBarButtonClickEvent;
class ToolBarDropDown;

		}
	}

	namespace render
	{

class Edge;
class External;
class INodeFacade;
class Node;
class QuickMenuTool;
class ShaderDependencyPane;
class ShaderGraph;
class ShaderViewer;

class T_DLLEXPORT ShaderGraphEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	ShaderGraphEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) T_OVERRIDE T_FINAL;

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE T_FINAL;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) T_OVERRIDE T_FINAL;

	void createEditorGraph();

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< ShaderGraph > m_shaderGraph;
	Ref< ui::custom::ToolBar > m_toolBar;
	Ref< ui::custom::ToolBarDropDown > m_toolPlatform;
	Ref< ui::custom::GraphControl > m_editorGraph;
	Ref< ShaderDependencyPane > m_dependencyPane;
	Ref< ShaderViewer > m_shaderViewer;
	Ref< ui::PopupMenu > m_menuPopup;
	Ref< QuickMenuTool > m_menuQuick;
	std::map< const TypeInfo*, Ref< INodeFacade > > m_nodeFacades;
	bool m_lastValidationResult;

	void createEditorNodes(const RefArray< Node >& shaderNodes, const RefArray< Edge >& shaderEdges);

	Ref< ui::custom::Node > createEditorNode(Node* shaderNode);

	void createNode(const TypeInfo* nodeType, const ui::Point& at);

	void refreshGraph();

	void updateGraph();

	void updateExternalNode(External* external);

	void eventToolClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventButtonDown(ui::MouseButtonDownEvent* event);

	void eventSelect(ui::custom::SelectEvent* event);

	void eventNodeMoved(ui::custom::NodeMovedEvent* event);

	void eventNodeDoubleClick(ui::custom::NodeActivateEvent* event);

	void eventEdgeConnect(ui::custom::EdgeConnectEvent* event);

	void eventEdgeDisconnect(ui::custom::EdgeDisconnectEvent* event);
};

	}
}

#endif	// traktor_render_ShaderGraphEditorPage_H
