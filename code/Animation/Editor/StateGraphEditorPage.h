#ifndef traktor_animation_StateGraphEditorPage_H
#define traktor_animation_StateGraphEditorPage_H

#include <map>
#include "Core/RefArray.h"
#include "Editor/IEditorPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

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

class StateGraph;
class State;
class Transition;

class T_DLLEXPORT StateGraphEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	StateGraphEditorPage(editor::IEditor* editor);

	virtual bool create(ui::Container* parent, editor::IEditorPageSite* site);

	virtual void destroy();

	virtual void activate();

	virtual void deactivate();

	virtual	bool setDataObject(db::Instance* instance, Object* data);

	virtual Ref< db::Instance > getDataInstance();

	virtual Ref< Object > getDataObject();

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position);

	virtual bool handleCommand(const ui::Command& command);

	virtual void handleDatabaseEvent(const Guid& eventId);

private:
	editor::IEditor* m_editor;
	Ref< editor::IEditorPageSite > m_site;
	Ref< db::Instance > m_stateGraphInstance;
	Ref< StateGraph > m_stateGraph;
	Ref< ui::custom::ToolBar > m_toolBar;
	Ref< ui::custom::GraphControl > m_editorGraph;
	Ref< ui::PopupMenu > m_menuPopup;

	void createEditorNodes(const RefArray< State >& states, const RefArray< Transition >& transitions);

	Ref< ui::custom::Node > createEditorNode(State* state);

	void createState(const ui::Point& at);

	void updateGraph();

	void eventToolClick(ui::Event* event);

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
