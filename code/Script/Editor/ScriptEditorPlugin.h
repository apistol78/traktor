/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef traktor_script_ScriptEditorPlugin_H
#define traktor_script_ScriptEditorPlugin_H

#include "Core/Ref.h"
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

class IScriptManager;
class ScriptDebuggerSessions;

/*! Editor scripting plugin.
 * \ingroup Script
 */
class ScriptEditorPlugin
:	public editor::IEditorPlugin
,	public IScriptDebuggerSessions::IListener
{
	T_RTTI_CLASS;

public:
	ScriptEditorPlugin(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, editor::IEditorPageSite* site) override final;

	virtual void destroy() override final;

	virtual bool handleCommand(const ui::Command& command, bool result) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

	virtual void handleWorkspaceOpened() override final;

	virtual void handleWorkspaceClosed() override final;

	/*! IScriptDebuggerSessions::IListener */
	// \{

	virtual void notifyBeginSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler) override final;

	virtual void notifyEndSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler) override final;

	virtual void notifySetBreakpoint(const Guid& scriptId, int32_t lineNumber) override final;

	virtual void notifyRemoveBreakpoint(const Guid& scriptId, int32_t lineNumber) override final;

	// \}

private:
	editor::IEditor* m_editor;
	Ref< script::IScriptManager > m_scriptManager;
	Ref< script::ScriptDebuggerSessions > m_debuggerSessions;
};

	}
}

#endif	// traktor_script_ScriptEditorPlugin_H

