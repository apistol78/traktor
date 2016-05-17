#ifndef traktor_script_ScriptDebuggerDialog_H
#define traktor_script_ScriptDebuggerDialog_H

#include "Script/Editor/IScriptDebuggerSessions.h"
#include "Ui/Dialog.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class Tab;

		namespace custom
		{
		}
	}

	namespace script
	{

class IScriptDebuggerSessions;

class ScriptDebuggerDialog
:	public ui::Dialog
,	public IScriptDebuggerSessions::IListener
{
	T_RTTI_CLASS;

public:
	ScriptDebuggerDialog(editor::IEditor* editor);

	virtual void destroy() T_OVERRIDE T_FINAL;

	bool create(ui::Widget* parent);

private:
	editor::IEditor* m_editor;
	Ref< IScriptDebuggerSessions > m_scriptDebuggerSessions;
	Ref< ui::Tab > m_tabSessions;

	/*! \name IScriptDebuggerSessions::IListener */
	/*! \{ */

	virtual void notifyBeginSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler) T_OVERRIDE T_FINAL;

	virtual void notifyEndSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler) T_OVERRIDE T_FINAL;

	virtual void notifySetBreakpoint(const Guid& scriptId, int32_t lineNumber) T_OVERRIDE T_FINAL;

	virtual void notifyRemoveBreakpoint(const Guid& scriptId, int32_t lineNumber) T_OVERRIDE T_FINAL;

	/*! \} */
};

	}
}

#endif	// traktor_script_ScriptDebuggerDialog_H
