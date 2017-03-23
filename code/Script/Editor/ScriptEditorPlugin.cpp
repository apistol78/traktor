#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/IEditor.h"
#include "Script/Editor/ScriptEditorPlugin.h"

namespace traktor
{
	namespace script
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptEditorPlugin", ScriptEditorPlugin, editor::IEditorPlugin)
	
ScriptEditorPlugin::ScriptEditorPlugin(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool ScriptEditorPlugin::create(ui::Widget* parent, editor::IEditorPageSite* site)
{
	return true;
}

void ScriptEditorPlugin::destroy()
{
}

bool ScriptEditorPlugin::handleCommand(const ui::Command& command, bool result)
{
	return false;
}

void ScriptEditorPlugin::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void ScriptEditorPlugin::handleWorkspaceOpened()
{
	IScriptDebuggerSessions* scriptDebuggerSessions = m_editor->getStoreObject< IScriptDebuggerSessions >(L"ScriptDebuggerSessions");
	if (scriptDebuggerSessions)
		scriptDebuggerSessions->addListener(this);
}

void ScriptEditorPlugin::handleWorkspaceClosed()
{
	IScriptDebuggerSessions* scriptDebuggerSessions = m_editor->getStoreObject< IScriptDebuggerSessions >(L"ScriptDebuggerSessions");
	if (scriptDebuggerSessions)
		scriptDebuggerSessions->removeListener(this);
}

void ScriptEditorPlugin::notifyBeginSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler)
{
	bool autoOpenDebugger = m_editor->getSettings()->getProperty< PropertyBoolean >(L"Editor.AutoOpenDebugger", false);
	if (autoOpenDebugger)
		m_editor->openTool(L"traktor.script.ScriptDebuggerTool", L"");
}

void ScriptEditorPlugin::notifyEndSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler)
{
}

void ScriptEditorPlugin::notifySetBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
}

void ScriptEditorPlugin::notifyRemoveBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
}

	}
}
