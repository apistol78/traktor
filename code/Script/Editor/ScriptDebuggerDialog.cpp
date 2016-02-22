#include "Core/Misc/SafeDestroy.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Script/Editor/IScriptDebuggerSessions.h"
#include "Script/Editor/ScriptDebuggerDialog.h"
#include "Script/Editor/ScriptDebuggerView.h"
#include "Ui/Application.h"
#include "Ui/FloodLayout.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptDebuggerDialog", ScriptDebuggerDialog, ui::Dialog)

ScriptDebuggerDialog::ScriptDebuggerDialog(editor::IEditor* editor)
:	m_editor(editor)
{
}

void ScriptDebuggerDialog::destroy()
{
	if (m_scriptDebuggerSessions)
		m_scriptDebuggerSessions->removeListener(this);

	ui::Dialog::destroy();
}

bool ScriptDebuggerDialog::create(ui::Widget* parent)
{
	if (!ui::Dialog::create(parent, i18n::Text(L"SCRIPT_DEBUGGER_TOOL"), ui::scaleBySystemDPI(1024), ui::scaleBySystemDPI(800), ui::Dialog::WsDefaultResizable, new ui::FloodLayout()))
		return false;

	m_tabSessions = new ui::Tab();
	m_tabSessions->create(this, ui::WsNone);

	m_scriptDebuggerSessions = m_editor->getStoreObject< IScriptDebuggerSessions >(L"ScriptDebuggerSessions");
	if (m_scriptDebuggerSessions)
		m_scriptDebuggerSessions->addListener(this);

	addEventHandler< ui::CloseEvent >(this, &ScriptDebuggerDialog::eventClose);
	update();
	return true;
}

void ScriptDebuggerDialog::notifyBeginSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler)
{
	Ref< ui::TabPage > tabPageSession = new ui::TabPage();
	tabPageSession->create(m_tabSessions, L"Session 0", new ui::FloodLayout());

	Ref< ScriptDebuggerView > debuggerView = new ScriptDebuggerView(m_editor, scriptDebugger);
	debuggerView->create(tabPageSession);

	tabPageSession->setData(L"DEBUGGER", scriptDebugger);
	tabPageSession->setData(L"VIEW", debuggerView);

	m_tabSessions->addPage(tabPageSession);
	m_tabSessions->setActivePage(tabPageSession);
	m_tabSessions->update();
}

void ScriptDebuggerDialog::notifyEndSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler)
{
	int32_t pageCount = m_tabSessions->getPageCount();
	for (int32_t i = 0; i < pageCount; ++i)
	{
		ui::TabPage* tabPageSession = m_tabSessions->getPage(i);
		T_ASSERT (tabPageSession);

		if (tabPageSession->getData< IScriptDebugger >(L"DEBUGGER") == scriptDebugger)
		{
			m_tabSessions->removePage(tabPageSession);
			safeDestroy(tabPageSession);
			break;
		}
	}
	m_tabSessions->update();
}

void ScriptDebuggerDialog::notifySetBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
}

void ScriptDebuggerDialog::notifyRemoveBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
}

void ScriptDebuggerDialog::eventClose(ui::CloseEvent* event)
{
	destroy();
}

	}
}
