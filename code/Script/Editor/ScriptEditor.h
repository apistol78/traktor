#ifndef traktor_script_ScriptEditor_H
#define traktor_script_ScriptEditor_H

#include <list>
#include "Editor/IObjectEditor.h"
#include "Script/IScriptManager.h"
#include "Script/Editor/IScriptDebuggerSessions.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxTypes.h"

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

class IEditor;

	}

	namespace ui
	{

class Event;
class ListBox;
class Tab;

		namespace custom
		{

class GridView;
class Splitter;
class SyntaxRichEdit;
class StatusBar;
class ToolBar;

		}
	}

	namespace script
	{

class IScriptContext;
class IScriptManager;
class Script;

class T_DLLCLASS ScriptEditor
:	public editor::IObjectEditor
,	public IErrorCallback
,	public IScriptDebuggerSessions::IListener
{
	T_RTTI_CLASS;

public:
	ScriptEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object);

	virtual void destroy();

	virtual void apply();

	virtual bool handleCommand(const ui::Command& command);

	virtual ui::Size getPreferredSize() const;

private:
	editor::IEditor* m_editor;
	Ref< db::Instance > m_instance;
	Ref< Script > m_script;
	Ref< IScriptDebuggerSessions > m_scriptDebuggerSessions;
	Ref< IScriptManager > m_scriptManager;
	Ref< ui::custom::Splitter > m_splitter;
	Ref< ui::custom::GridView > m_outlineGrid;
	Ref< ui::ListBox > m_dependencyList;
	Ref< ui::custom::SyntaxRichEdit > m_edit;
	Ref< ui::custom::StatusBar > m_compileStatus;
	Ref< ui::Tab > m_tabSessions;
	std::list< ui::custom::SyntaxOutline > m_outline;
	int32_t m_compileCountDown;


	/*! \name IErrorCallback */
	/*! \{ */

	virtual void syntaxError(uint32_t line, const std::wstring& message);

	virtual void otherError(const std::wstring& message);

	/*! \} */


	/*! \name IScriptDebuggerSessions::IListener */
	/*! \{ */

	virtual void notifyDebuggerBeginSession(IScriptDebugger* scriptDebugger);

	virtual void notifyDebuggerEndSession(IScriptDebugger* scriptDebugger);

	virtual void notifyDebuggerSetBreakpoint(const Guid& scriptId, int32_t lineNumber);

	virtual void notifyDebuggerRemoveBreakpoint(const Guid& scriptId, int32_t lineNumber);

	/*! \} */


	void updateDependencyList();

	void eventOutlineDoubleClick(ui::Event* event);

	void eventDependencyToolClick(ui::Event* event);

	void eventDependencyListDoubleClick(ui::Event* event);

	void eventDebuggerToolClick(ui::Event* event);

	void eventScriptChange(ui::Event* event);

	void eventScriptDoubleClick(ui::Event* event);

	void eventTimer(ui::Event* event);

	void eventBreakPoint(ui::Event* event);
};

	}
}

#endif	// traktor_script_ScriptEditor_H
