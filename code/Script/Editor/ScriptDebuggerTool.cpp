/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
#include "Script/Editor/ScriptDebuggerTool.h"
#include "Script/Editor/ScriptDebuggerDialog.h"
#include "I18N/Text.h"
#include "Ui/StyleBitmap.h"

namespace traktor
{
	namespace script
	{


T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptDebuggerTool", 0, ScriptDebuggerTool, IEditorTool)

std::wstring ScriptDebuggerTool::getDescription() const
{
	return i18n::Text(L"SCRIPT_DEBUGGER_TOOL");
}

Ref< ui::IBitmap > ScriptDebuggerTool::getIcon() const
{
	return new ui::StyleBitmap(L"Script.Tool.Debugger");
}

bool ScriptDebuggerTool::launch(ui::Widget* parent, editor::IEditor* script, const std::wstring& param)
{
	// If debugger dialog is already created then we only need to show it.
	if (m_debuggerDialog)
	{
		m_debuggerDialog->show();
		return true;
	}

	// Create dialog and register ourself for close events.
	m_debuggerDialog = new ScriptDebuggerDialog(script);
	if (m_debuggerDialog->create(parent))
	{
		m_debuggerDialog->show();
		m_debuggerDialog->addEventHandler< ui::CloseEvent >(this, &ScriptDebuggerTool::eventCloseDialog);
	}
	else
		safeDestroy(m_debuggerDialog);

	return true;
}

void ScriptDebuggerTool::eventCloseDialog(ui::CloseEvent* event)
{
	safeDestroy(m_debuggerDialog);
}

	}
}
