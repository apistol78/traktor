#include "Core/Misc/String.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPage.h"
#include "I18N/Text.h"
#include "Script/LocalComposite.h"
#include "Script/LocalSimple.h"
#include "Script/StackFrame.h"
#include "Script/Editor/ScriptBreakpointEvent.h"
#include "Script/Editor/ScriptDebuggerView.h"
#include "Ui/Application.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptDebuggerView", ScriptDebuggerView, ui::Container)

ScriptDebuggerView::ScriptDebuggerView(editor::IEditor* editor, IScriptDebugger* scriptDebugger)
:	m_editor(editor)
,	m_scriptDebugger(scriptDebugger)
{
}

ScriptDebuggerView::~ScriptDebuggerView()
{
	T_ASSERT (!m_scriptDebugger);
}

bool ScriptDebuggerView::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%,*", 0, 0)))
		return false;

	m_debuggerTools = new ui::custom::ToolBar();
	if (!m_debuggerTools->create(this))
		return false;

	m_debuggerTools->addImage(new ui::StyleBitmap(L"Script.DebugContinue"), 1);
	m_debuggerTools->addImage(new ui::StyleBitmap(L"Script.DebugStepInto"), 1);
	m_debuggerTools->addImage(new ui::StyleBitmap(L"Script.DebugStepOver"), 1);
	m_debuggerTools->addImage(new ui::StyleBitmap(L"Script.DebugStop"), 1);
	m_debuggerTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_CONTINUE"), 0, ui::Command(L"Script.Editor.Continue")));
	m_debuggerTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_BREAK"), 3, ui::Command(L"Script.Editor.Break")));
	m_debuggerTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_STEP_INTO"), 1, ui::Command(L"Script.Editor.StepInto")));
	m_debuggerTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_STEP_OVER"), 2, ui::Command(L"Script.Editor.StepOver")));
	m_debuggerTools->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &ScriptDebuggerView::eventDebuggerToolClick);

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(this, true, 50, true);

	m_callStackGrid = new ui::custom::GridView();
	m_callStackGrid->create(splitter, ui::WsDoubleBuffer | ui::custom::GridView::WsColumnHeader);
	m_callStackGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_EDITOR_DEBUG_FUNCTION"), ui::scaleBySystemDPI(180)));
	m_callStackGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_EDITOR_DEBUG_LINE"), ui::scaleBySystemDPI(100)));
	m_callStackGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_EDITOR_DEBUG_SCRIPT"), ui::scaleBySystemDPI(200)));
	m_callStackGrid->setEnable(false);
	m_callStackGrid->addEventHandler< ui::MouseDoubleClickEvent >(this, &ScriptDebuggerView::eventCallStackGridDoubleClick);

	m_localsGrid = new ui::custom::GridView();
	m_localsGrid->create(splitter, ui::WsDoubleBuffer | ui::custom::GridView::WsColumnHeader);
	m_localsGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_EDITOR_DEBUG_LOCAL_NAME"), ui::scaleBySystemDPI(120)));
	m_localsGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_EDITOR_DEBUG_LOCAL_VALUE"), ui::scaleBySystemDPI(300)));
	m_localsGrid->setEnable(false);

	m_scriptDebugger->addListener(this);
	return true;
}

void ScriptDebuggerView::destroy()
{
	if (m_scriptDebugger)
	{
		m_scriptDebugger->removeListener(this);
		m_scriptDebugger = 0;
	}
	ui::Container::destroy();
}

bool ScriptDebuggerView::handleCommand(const ui::Command& command)
{
	if (command == L"Script.Editor.Continue")
	{
		m_callStackGrid->setEnable(false);
		m_callStackGrid->update();

		m_localsGrid->setEnable(false);
		m_localsGrid->update();

		m_scriptDebugger->actionContinue();
	}
	else if (command == L"Script.Editor.Break")
		m_scriptDebugger->actionBreak();
	else if (command == L"Script.Editor.StepInto")
		m_scriptDebugger->actionStepInto();
	else if (command == L"Script.Editor.StepOver")
		m_scriptDebugger->actionStepOver();
	else
		return false;

	return true;
}

Ref< ui::custom::GridRow > ScriptDebuggerView::createVariableRow(const script::Local* local)
{
	Ref< ui::custom::GridRow > row = new ui::custom::GridRow(0);
	row->add(new ui::custom::GridItem(local->getName()));
	if (const script::LocalComposite* composite = dynamic_type_cast< const script::LocalComposite* >(local))
	{
		if (!composite->getValue().empty())
			row->add(new ui::custom::GridItem(composite->getValue()));

		const RefArray< script::Local >& values = composite->getValues();
		for (RefArray< script::Local >::const_iterator j = values.begin(); j != values.end(); ++j)
		{
			Ref< ui::custom::GridRow > child = createVariableRow(*j);
			if (child)
				row->addChild(child);
		}
	}
	else if (const script::LocalSimple* simple = dynamic_type_cast< const script::LocalSimple* >(local))
		row->add(new ui::custom::GridItem(simple->getValue()));
	return row;
}

void ScriptDebuggerView::updateLocals(int32_t depth)
{
	m_localsGrid->setEnable(true);
	m_localsGrid->removeAllRows();

	if (depth >= 0 && depth < m_stackFrames.size())
	{
		const StackFrame* sf = m_stackFrames[depth];
		const RefArray< Local >& locals = sf->getLocals();

		for (RefArray< script::Local >::const_iterator j = locals.begin(); j != locals.end(); ++j)
		{
			Ref< ui::custom::GridRow > row = createVariableRow(*j);
			if (row)
				m_localsGrid->addRow(row);
		}
	}

	m_localsGrid->update();
}

void ScriptDebuggerView::breakpointReached(IScriptDebugger* scriptDebugger)
{
	m_stackFrames.resize(0);
	for (uint32_t depth = 0; ; ++depth)
	{
		Ref< StackFrame > sf = scriptDebugger->captureStackFrame(depth);
		if (!sf)
			break;

		m_stackFrames.push_back(sf);
	}

	m_callStackGrid->setEnable(true);
	m_callStackGrid->removeAllRows();

	int32_t depth = 0;

	for (RefArray< StackFrame >::const_iterator i = m_stackFrames.begin(); i != m_stackFrames.end(); ++i)
	{
		Ref< db::Instance > scriptInstance = m_editor->getSourceDatabase()->getInstance((*i)->getScriptId());

		Ref< ui::custom::GridRow > row = new ui::custom::GridRow(0);

		row->add(new ui::custom::GridItem((*i)->getFunctionName()));
		row->add(new ui::custom::GridItem(toString((*i)->getLine() + 1)));
		row->add(new ui::custom::GridItem(scriptInstance ? scriptInstance->getName() : L"(Unknown script)"));
		row->setData(L"SCRIPT_ID", new PropertyString((*i)->getScriptId().format()));
		row->setData(L"SCRIPT_LINE", new PropertyInteger((*i)->getLine()));
		row->setData(L"FRAME_DEPTH", new PropertyInteger(depth++));

		m_callStackGrid->addRow(row);
	}

	updateLocals(0);

	// Issue event to notify script editor about breakpoint in this view.
	ScriptBreakpointEvent eventBreakPoint(this, !m_stackFrames.empty() ? m_stackFrames.front() : 0);
	raiseEvent(&eventBreakPoint);
}

void ScriptDebuggerView::eventDebuggerToolClick(ui::custom::ToolBarButtonClickEvent* event)
{
	handleCommand(event->getCommand());
}

void ScriptDebuggerView::eventCallStackGridDoubleClick(ui::MouseDoubleClickEvent* event)
{
	if (!m_callStackGrid->isEnable())
		return;

	ui::custom::GridRow* selectedRow = m_callStackGrid->getSelectedRow();
	if (selectedRow)
	{
		Guid scriptId = Guid(*(selectedRow->getData< PropertyString >(L"SCRIPT_ID")));
		int32_t line = *(selectedRow->getData< PropertyInteger >(L"SCRIPT_LINE"));
		int32_t depth = *(selectedRow->getData< PropertyInteger >(L"FRAME_DEPTH"));

		Ref< db::Instance > scriptInstance = m_editor->getSourceDatabase()->getInstance(scriptId);
		if (scriptInstance)
		{
			m_editor->openEditor(scriptInstance);

			editor::IEditorPage* activeEditorPage = m_editor->getActiveEditorPage();
			if (activeEditorPage)
				activeEditorPage->handleCommand(ui::Command(
					line,
					L"Script.Editor.GotoLine"
				));
		}

		updateLocals(depth);
	}
	else
		updateLocals(0);
}

	}
}
