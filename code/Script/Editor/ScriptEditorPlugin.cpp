/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/IRuntimeClassFactory.h"
#include "Core/Class/OrderedClassRegistrar.h"
#include "Core/Misc/ObjectStore.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/IEditor.h"
#include "Script/IScriptManager.h"
#include "Script/Editor/ScriptDebuggerSessions.h"
#include "Script/Editor/ScriptEditorPlugin.h"

namespace traktor::script
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptEditorPlugin", 0, ScriptEditorPlugin, editor::IEditorPlugin)

bool ScriptEditorPlugin::create(editor::IEditor* editor, ui::Widget* parent, editor::IEditorPageSite* site)
{
	m_editor = editor;

	const TypeInfo* scriptManagerType = TypeInfo::find(L"traktor.script.ScriptManagerLua");
	if (scriptManagerType)
	{
		m_scriptManager = mandatory_non_null_type_cast< IScriptManager* >(scriptManagerType->createInstance());
		if (!m_scriptManager)
			return false;

		// Register all runtime classes, first collect all classes
		// and then register them in class dependency order.
		OrderedClassRegistrar registrar;
		for (const auto runtimeClassFactoryType : type_of< IRuntimeClassFactory >().findAllOf(false))
		{
			Ref< IRuntimeClassFactory > runtimeClassFactory = dynamic_type_cast< IRuntimeClassFactory* >(runtimeClassFactoryType->createInstance());
			if (runtimeClassFactory)
				runtimeClassFactory->createClasses(&registrar);
		}
		registrar.registerClassesInOrder(m_scriptManager);

		// Expose script manager to other editors.
		m_editor->getObjectStore()->set(m_scriptManager);
	}

	// Create target script debugger dispatcher.
	m_debuggerSessions = new script::ScriptDebuggerSessions();
	m_editor->getObjectStore()->set(m_debuggerSessions);

	return true;
}

void ScriptEditorPlugin::destroy()
{
	m_editor->getObjectStore()->unset(m_debuggerSessions);
	m_editor->getObjectStore()->unset(m_scriptManager);
	safeDestroy(m_scriptManager);
}

int32_t ScriptEditorPlugin::getOrdinal() const
{
	return 0;
}

void ScriptEditorPlugin::getCommands(std::list< ui::Command >& outCommands) const
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
	IScriptDebuggerSessions* scriptDebuggerSessions = m_editor->getObjectStore()->get< IScriptDebuggerSessions >();
	if (scriptDebuggerSessions)
		scriptDebuggerSessions->addListener(this);
}

void ScriptEditorPlugin::handleWorkspaceClosed()
{
	IScriptDebuggerSessions* scriptDebuggerSessions = m_editor->getObjectStore()->get< IScriptDebuggerSessions >();
	if (scriptDebuggerSessions)
		scriptDebuggerSessions->removeListener(this);
}

void ScriptEditorPlugin::handleEditorClosed()
{
	if (m_scriptManager)
		m_scriptManager->collectGarbage(true);
}

void ScriptEditorPlugin::notifyBeginSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler)
{
	bool autoOpenDebugger = m_editor->getSettings()->getProperty< bool >(L"Editor.AutoOpenDebugger", false);
	if (autoOpenDebugger)
		m_editor->openTool(L"traktor.script.ScriptDebuggerTool", nullptr);
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
