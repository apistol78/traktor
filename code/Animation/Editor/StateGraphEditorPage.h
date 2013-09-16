#ifndef traktor_animation_StateGraphEditorPage_H
#define traktor_animation_StateGraphEditorPage_H

#include <map>
#include "Core/RefArray.h"
#include "Editor/IEditorPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
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
class PopupMenu;
class Point;

		namespace custom
		{

class ToolBar;
class GraphControl;
class Node;

		}
	}

	namespace animation
	{

class AnimationPreviewControl;
class StateGraph;
class StateNode;
class Transition;

class T_DLLEXPORT StateGraphEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	StateGraphEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent);

	virtual void destroy();

	virtual void activate();

	virtual void deactivate();

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position);

	virtual bool handleCommand(const ui::Command& command);

	virtual void handleDatabaseEvent(const Guid& eventId);

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< StateGraph > m_stateGraph;
	Ref< ui::custom::ToolBar > m_toolBarGraph;
	Ref< ui::custom::GraphControl > m_editorGraph;
	Ref< ui::PopupMenu > m_menuPopup;
	Ref< ui::Container > m_containerPreview;
	Ref< ui::custom::ToolBar > m_toolBarPreview;
	Ref< AnimationPreviewControl > m_previewControl;

	void bindStateNodes();

	void createEditorNodes(const RefArray< StateNode >& states, const RefArray< Transition >& transitions);

	Ref< ui::custom::Node > createEditorNode(StateNode* state);

	void createState(const ui::Point& at);

	void updateGraph();

	void eventToolBarGraphClick(ui::Event* event);

	void eventToolBarPreviewClick(ui::Event* event);

	void eventPropertyChange(ui::Event* event);

	void eventButtonDown(ui::Event* event);

	void eventSelect(ui::Event* event);

	void eventNodeMoved(ui::Event* event);

	void eventEdgeConnect(ui::Event* event);

	void eventEdgeDisconnect(ui::Event* event);
};

	}
}

#endif	// traktor_animation_StateGraphEditorPage_H
