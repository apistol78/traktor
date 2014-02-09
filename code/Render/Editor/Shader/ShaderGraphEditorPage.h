#ifndef traktor_render_ShaderGraphEditorPage_H
#define traktor_render_ShaderGraphEditorPage_H

#include <map>
#include "Core/RefArray.h"
#include "Editor/IEditorPage.h"

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
class Event;
class Point;
class PopupMenu;

		namespace custom
		{

class GraphControl;
class Node;
class ToolBar;
class ToolBarDropDown;

		}
	}

	namespace render
	{

class Edge;
class External;
class Node;
class NodeFacade;
class QuickMenuTool;
class ShaderDependencyPane;
class ShaderGraph;
class ShaderViewer;

class T_DLLEXPORT ShaderGraphEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	ShaderGraphEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent);

	virtual void destroy();

	virtual void activate();

	virtual void deactivate();

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position);

	virtual bool handleCommand(const ui::Command& command);

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId);

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
	std::map< const TypeInfo*, Ref< NodeFacade > > m_nodeFacades;
	bool m_lastValidationResult;

	void createEditorNodes(const RefArray< Node >& shaderNodes, const RefArray< Edge >& shaderEdges);

	Ref< ui::custom::Node > createEditorNode(Node* shaderNode);

	void createNode(const TypeInfo* nodeType, const ui::Point& at);

	void refreshGraph();

	void updateGraph();

	void updateExternalNode(External* external);

	void eventToolClick(ui::Event* event);

	void eventPropertyChange(ui::Event* event);

	void eventButtonDown(ui::Event* event);

	void eventSelect(ui::Event* event);

	void eventNodeMoved(ui::Event* event);

	void eventNodeDoubleClick(ui::Event* event);

	void eventEdgeConnect(ui::Event* event);

	void eventEdgeDisconnect(ui::Event* event);
};

	}
}

#endif	// traktor_render_ShaderGraphEditorPage_H
