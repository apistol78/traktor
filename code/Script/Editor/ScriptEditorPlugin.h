#ifndef traktor_script_ScriptEditorPlugin_H
#define traktor_script_ScriptEditorPlugin_H

#include "Editor/IEditorPlugin.h"
#include "Script/Editor/IScriptDebuggerSessions.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace script
	{
	
/*! \brief Editor scripting plugin.
 * \ingroup Script
 */
class ScriptEditorPlugin
:	public editor::IEditorPlugin
,	public IScriptDebuggerSessions::IListener
{
	T_RTTI_CLASS;

public:
	ScriptEditorPlugin(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, editor::IEditorPageSite* site) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool handleCommand(const ui::Command& command, bool result) T_OVERRIDE T_FINAL;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) T_OVERRIDE T_FINAL;

	virtual void handleWorkspaceOpened() T_OVERRIDE T_FINAL;

	virtual void handleWorkspaceClosed() T_OVERRIDE T_FINAL;

	/*! IScriptDebuggerSessions::IListener */
	// \{

	virtual void notifyBeginSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler) T_OVERRIDE T_FINAL;

	virtual void notifyEndSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler) T_OVERRIDE T_FINAL;

	virtual void notifySetBreakpoint(const Guid& scriptId, int32_t lineNumber) T_OVERRIDE T_FINAL;

	virtual void notifyRemoveBreakpoint(const Guid& scriptId, int32_t lineNumber) T_OVERRIDE T_FINAL;

	// \}

private:
	editor::IEditor* m_editor;
};
	
	}
}

#endif	// traktor_script_ScriptEditorPlugin_H

