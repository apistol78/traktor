#include "Core/Misc/SafeDestroy.h"
#include "Script/Editor/ScriptDebuggerTool.h"
#include "Script/Editor/ScriptDebuggerDialog.h"
#include "I18N/Text.h"

namespace traktor
{
	namespace script
	{


T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptDebuggerTool", 0, ScriptDebuggerTool, IEditorTool)

std::wstring ScriptDebuggerTool::getDescription() const
{
	return i18n::Text(L"SCRIPT_DEBUGGER_TOOL");
}

bool ScriptDebuggerTool::launch(ui::Widget* parent, editor::IEditor* script)
{
	safeDestroy(m_debuggerDialog);

	m_debuggerDialog = new ScriptDebuggerDialog(script);
	if (m_debuggerDialog->create(parent))
		m_debuggerDialog->show();
	else
		safeDestroy(m_debuggerDialog);

	return true;
}

	}
}
