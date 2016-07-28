#ifndef traktor_script_ScriptEditorPage_H
#define traktor_script_ScriptEditorPage_H

#include <list>
#include "Editor/IEditorPage.h"
#include "Script/IScriptDebugger.h"
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

class IBitmap;
class ListBox;
class PopupMenu;
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
class ScriptAsset;
class ScriptBreakpointEvent;
class ScriptClassesView;
class SearchControl;
class SearchEvent;

class T_DLLCLASS ScriptEditorPage
:	public editor::IEditorPage
,	public IErrorCallback
,	public IScriptDebugger::IListener
,	public IScriptDebuggerSessions::IListener
{
	T_RTTI_CLASS;

public:
	ScriptEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void activate() T_OVERRIDE T_FINAL;

	virtual void deactivate() T_OVERRIDE T_FINAL;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) T_OVERRIDE T_FINAL;

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE T_FINAL;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) T_OVERRIDE T_FINAL;

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;

	std::wstring m_assetPath;
	Ref< Script > m_script;
	Ref< ScriptAsset > m_scriptAsset;
	Ref< IScriptDebuggerSessions > m_scriptDebuggerSessions;
	Ref< IScriptManager > m_scriptManager;
	Ref< IScriptOutline > m_scriptOutline;
	Ref< Preprocessor > m_preprocessor;
	Ref< ui::IBitmap > m_bitmapFunction;
	Ref< ui::IBitmap > m_bitmapFunctionLocal;
	Ref< ui::IBitmap > m_bitmapFunctionReference;
	Ref< ui::Container > m_containerExplorer;
	Ref< ui::custom::GridView > m_outlineGrid;
	Ref< ScriptClassesView > m_classesView;
	Ref< ui::custom::SyntaxRichEdit > m_edit;
	Ref< ui::PopupMenu > m_editMenu;
	Ref< ui::custom::StatusBar > m_compileStatus;
	Ref< SearchControl > m_searchControl;
	std::wstring m_findNeedle;
	std::wstring m_replaceValue;
	int32_t m_compileCountDown;
	int32_t m_foundLineAttribute;
	int32_t m_debugLineAttribute;
	int32_t m_debugLineLast;

	/*! \name IErrorCallback */
	/*! \{ */

	virtual void syntaxError(const std::wstring& name, uint32_t line, const std::wstring& message) T_OVERRIDE T_FINAL;

	virtual void otherError(const std::wstring& message) T_OVERRIDE T_FINAL;

	/*! \} */

	/*! \name IScriptDebugger::IListener */
	/*! \{ */

	virtual void debugeeStateChange(IScriptDebugger* scriptDebugger) T_OVERRIDE T_FINAL;

	/*! \} */

	/*! \name IScriptDebuggerSessions::IListener */
	/*! \{ */

	virtual void notifyBeginSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler) T_OVERRIDE T_FINAL;

	virtual void notifyEndSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler) T_OVERRIDE T_FINAL;

	virtual void notifySetBreakpoint(const Guid& scriptId, int32_t lineNumber) T_OVERRIDE T_FINAL;

	virtual void notifyRemoveBreakpoint(const Guid& scriptId, int32_t lineNumber) T_OVERRIDE T_FINAL;

	/*! \} */

	void updateBreakpoints();

	void buildOutlineGrid(ui::custom::GridView* grid, ui::custom::GridRow* parent, const IScriptOutline::Node* on);

	void eventOutlineDoubleClick(ui::MouseDoubleClickEvent* event);

	void eventToolBarEditClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventScriptChange(ui::ContentChangeEvent* event);

	void eventScriptButtonDown(ui::MouseButtonDownEvent* event);

	void eventScriptButtonUp(ui::MouseButtonUpEvent* event);

	void eventScriptSize(ui::SizeEvent* event);

	void eventSearch(SearchEvent* event);

	void eventTimer(ui::TimerEvent* event);
};

	}
}

#endif	// traktor_script_ScriptEditorPage_H
