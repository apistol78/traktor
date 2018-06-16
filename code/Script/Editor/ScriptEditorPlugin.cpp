/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/IRuntimeClassFactory.h"
#include "Core/Class/OrderedClassRegistrar.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/IEditor.h"
#include "Script/IScriptManager.h"
#include "Script/Editor/ScriptDebuggerSessions.h"
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
	const TypeInfo* scriptManagerType = TypeInfo::find(L"traktor.script.ScriptManagerLua");
	if (scriptManagerType)
	{
		m_scriptManager = mandatory_non_null_type_cast< IScriptManager* >(scriptManagerType->createInstance());
		if (!m_scriptManager)
			return false;

		// Register all runtime classes, first collect all classes
		// and then register them in class dependency order.
		OrderedClassRegistrar registrar;
		std::set< const TypeInfo* > runtimeClassFactoryTypes;
		type_of< IRuntimeClassFactory >().findAllOf(runtimeClassFactoryTypes, false);
		for (std::set< const TypeInfo* >::const_iterator i = runtimeClassFactoryTypes.begin(); i != runtimeClassFactoryTypes.end(); ++i)
		{
			Ref< IRuntimeClassFactory > runtimeClassFactory = dynamic_type_cast< IRuntimeClassFactory* >((*i)->createInstance());
			if (runtimeClassFactory)
				runtimeClassFactory->createClasses(&registrar);
		}
		registrar.registerClassesInOrder(m_scriptManager);

		// Expose script manager to other editors.
		m_editor->setStoreObject(L"ScriptManager", m_scriptManager);
	}

	// Create target script debugger dispatcher.
	m_debuggerSessions = new script::ScriptDebuggerSessions();
	m_editor->setStoreObject(L"ScriptDebuggerSessions", m_debuggerSessions);

	return true;
}

void ScriptEditorPlugin::destroy()
{
	m_editor->setStoreObject(L"ScriptDebuggerSessions", 0);
	m_editor->setStoreObject(L"ScriptManager", 0);
	safeDestroy(m_scriptManager);
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
	{
		// Add, always running, editor session.
		if (m_scriptManager)
		{
			Ref< IScriptDebugger > debugger = m_scriptManager->createDebugger();
			if (debugger)
				scriptDebuggerSessions->beginSession(debugger, 0);
		}

		// Register ourself after beginning editor session as we don't want "auto open" yet.
		scriptDebuggerSessions->addListener(this);
	}
}

void ScriptEditorPlugin::handleWorkspaceClosed()
{
	IScriptDebuggerSessions* scriptDebuggerSessions = m_editor->getStoreObject< IScriptDebuggerSessions >(L"ScriptDebuggerSessions");
	if (scriptDebuggerSessions)
		scriptDebuggerSessions->removeListener(this);
}

void ScriptEditorPlugin::notifyBeginSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler)
{
	bool autoOpenDebugger = m_editor->getSettings()->getProperty< bool >(L"Editor.AutoOpenDebugger", false);
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
