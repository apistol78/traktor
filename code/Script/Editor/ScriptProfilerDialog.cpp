#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Script/Editor/IScriptDebuggerSessions.h"
#include "Script/Editor/ScriptProfilerDialog.h"
#include "Script/Editor/ScriptProfilerView.h"
#include "Ui/Application.h"
#include "Ui/FloodLayout.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptProfilerDialog", ScriptProfilerDialog, ui::Dialog)

ScriptProfilerDialog::ScriptProfilerDialog(editor::IEditor* editor)
:	m_editor(editor)
{
}

void ScriptProfilerDialog::destroy()
{
	if (m_scriptDebuggerSessions)
		m_scriptDebuggerSessions->removeListener(this);

	ui::Dialog::destroy();
}

bool ScriptProfilerDialog::create(ui::Widget* parent)
{
	if (!ui::Dialog::create(parent, i18n::Text(L"SCRIPT_PROFILER_TOOL"), ui::scaleBySystemDPI(1024), ui::scaleBySystemDPI(800), ui::Dialog::WsDefaultResizable, new ui::FloodLayout()))
		return false;

	m_tabSessions = new ui::Tab();
	m_tabSessions->create(this);

	m_scriptDebuggerSessions = m_editor->getStoreObject< IScriptDebuggerSessions >(L"ScriptDebuggerSessions");
	if (m_scriptDebuggerSessions)
		m_scriptDebuggerSessions->addListener(this);

	addEventHandler< ui::CloseEvent >(this, &ScriptProfilerDialog::eventClose);
	update();

	return true;
}

void ScriptProfilerDialog::notifyBeginSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler)
{
	Ref< ui::TabPage > tabPageSession = new ui::TabPage();
	tabPageSession->create(m_tabSessions, L"Session 0", new ui::FloodLayout());

	Ref< ScriptProfilerView > profilerView = new ScriptProfilerView(m_editor, scriptProfiler);
	profilerView->create(tabPageSession);

	tabPageSession->setData(L"PROFILER", scriptProfiler);

	m_tabSessions->addPage(tabPageSession);
	m_tabSessions->setActivePage(tabPageSession);
	m_tabSessions->update();
}

void ScriptProfilerDialog::notifyEndSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler)
{
	int32_t pageCount = m_tabSessions->getPageCount();
	for (int32_t i = 0; i < pageCount; ++i)
	{
		ui::TabPage* tabPageSession = m_tabSessions->getPage(i);
		T_ASSERT (tabPageSession);

		if (tabPageSession->getData< IScriptProfiler >(L"PROFILER") == scriptProfiler)
		{
			tabPageSession->setText(L"Session 0 (Terminated)");
			break;
		}
	}
	m_tabSessions->update();
}

void ScriptProfilerDialog::notifySetBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
}

void ScriptProfilerDialog::notifyRemoveBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
}

void ScriptProfilerDialog::eventClose(ui::CloseEvent* event)
{
	destroy();
}

	}
}
