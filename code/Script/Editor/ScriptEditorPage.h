#ifndef traktor_script_ScriptEditorPage_H
#define traktor_script_ScriptEditorPage_H

#include <list>
#include "Editor/IEditorPage.h"
#include "Script/IScriptManager.h"
#include "Script/Editor/IScriptDebuggerSessions.h"
#include "Script/Editor/IScriptOutline.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxTypes.h"
#include "Ui/Events/AllEvents.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EDITOR_EXPORT)
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

class Bitmap;
class ListBox;
class Tab;

		namespace custom
		{

class GridRow;
class GridView;
class Splitter;
class SyntaxRichEdit;
class StatusBar;
class ToolBar;
class ToolBarButtonClickEvent;

		}
	}

	namespace script
	{

class IScriptContext;
class IScriptManager;
class Preprocessor;
class Script;
class ScriptBreakpointEvent;

class T_DLLCLASS ScriptEditorPage
:	public editor::IEditorPage
,	public IErrorCallback
,	public IScriptDebuggerSessions::IListener
{
	T_RTTI_CLASS;

public:
	ScriptEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

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

	Ref< Script > m_script;
	Ref< IScriptDebuggerSessions > m_scriptDebuggerSessions;
	Ref< IScriptManager > m_scriptManager;
	Ref< IScriptOutline > m_scriptOutline;
	Ref< Preprocessor > m_preprocessor;
	Ref< ui::Bitmap > m_bitmapFunction;
	Ref< ui::Bitmap > m_bitmapFunctionLocal;
	Ref< ui::Bitmap > m_bitmapFunctionReference;
	Ref< ui::Container > m_containerExplorer;
	Ref< ui::Container > m_containerDebugger;
	Ref< ui::custom::GridView > m_outlineGrid;
	Ref< ui::ListBox > m_dependencyList;
	Ref< ui::ListBox > m_dependentList;
	Ref< ui::custom::SyntaxRichEdit > m_edit;
	Ref< ui::custom::StatusBar > m_compileStatus;
	Ref< ui::Tab > m_tabSessions;
	std::wstring m_findNeedle;
	std::wstring m_replaceValue;
	int32_t m_compileCountDown;


	/*! \name IErrorCallback */
	/*! \{ */

	virtual void syntaxError(const std::wstring& name, uint32_t line, const std::wstring& message);

	virtual void otherError(const std::wstring& message);

	/*! \} */


	/*! \name IScriptDebuggerSessions::IListener */
	/*! \{ */

	virtual void notifyBeginSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler);

	virtual void notifyEndSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler);

	virtual void notifySetBreakpoint(const Guid& scriptId, int32_t lineNumber);

	virtual void notifyRemoveBreakpoint(const Guid& scriptId, int32_t lineNumber);

	/*! \} */


	void updateDependencyList();

	void updateDependentList();

	void buildOutlineGrid(ui::custom::GridView* grid, ui::custom::GridRow* parent, const IScriptOutline::Node* on);

	void eventOutlineDoubleClick(ui::MouseDoubleClickEvent* event);

	void eventDependencyToolClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventDependencyListDoubleClick(ui::MouseDoubleClickEvent* event);

	void eventDependentListDoubleClick(ui::MouseDoubleClickEvent* event);

	void eventToolBarEditClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventScriptChange(ui::ContentChangeEvent* event);

	void eventScriptDoubleClick(ui::MouseDoubleClickEvent* event);

	void eventTimer(ui::TimerEvent* event);

	void eventBreakPoint(ScriptBreakpointEvent* event);
};

	}
}

#endif	// traktor_script_ScriptEditorPage_H
