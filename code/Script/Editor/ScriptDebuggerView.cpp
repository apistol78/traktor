#include "Core/Misc/String.h"
#include "I18N/Text.h"
#include "Script/CallStack.h"
#include "Script/LocalComposite.h"
#include "Script/LocalSimple.h"
#include "Script/Editor/ScriptDebuggerView.h"
#include "Ui/Bitmap.h"
#include "Ui/MethodHandler.h"
#include "Ui/Tab.h"
#include "Ui/TableLayout.h"
#include "Ui/TabPage.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Events/CommandEvent.h"

// Resources
#include "Resources/Debug.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

enum { EiBreakPoint = ui::EiUser + 1 };

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptDebuggerView", ScriptDebuggerView, ui::Container)

ScriptDebuggerView::ScriptDebuggerView(IScriptDebugger* scriptDebugger)
:	m_scriptDebugger(scriptDebugger)
{
	m_scriptDebugger->addListener(this);
}

ScriptDebuggerView::~ScriptDebuggerView()
{
	m_scriptDebugger->removeListener(this);
}

bool ScriptDebuggerView::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%,*", 0, 0)))
		return false;

	m_debuggerTools = new ui::custom::ToolBar();
	if (!m_debuggerTools->create(this))
		return false;

	m_debuggerTools->addImage(ui::Bitmap::load(c_ResourceDebug, sizeof(c_ResourceDebug), L"png"), 4);
	m_debuggerTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_CONTINUE"), ui::Command(L"Script.Editor.Continue"), 1));
	m_debuggerTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_BREAK"), ui::Command(L"Script.Editor.Break"), 0));
	m_debuggerTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_STEP_INTO"), ui::Command(L"Script.Editor.StepInto"), 2));
	m_debuggerTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_STEP_OVER"), ui::Command(L"Script.Editor.StepOver"), 3));
	m_debuggerTools->addClickEventHandler(ui::createMethodHandler(this, &ScriptDebuggerView::eventDebuggerToolClick));

	Ref< ui::Tab > tabDebugger = new ui::Tab();
	tabDebugger->create(this, ui::WsNone);

	Ref< ui::TabPage > tabPageCallStack = new ui::TabPage();
	tabPageCallStack->create(tabDebugger, L"Call Stack", new ui::TableLayout(L"100%", L"100%", 0, 0));

	m_callStackGrid = new ui::custom::GridView();
	m_callStackGrid->create(tabPageCallStack, ui::WsDoubleBuffer | ui::custom::GridView::WsColumnHeader);
	m_callStackGrid->addColumn(new ui::custom::GridColumn(L"Function", 200));
	m_callStackGrid->addColumn(new ui::custom::GridColumn(L"Line", 100));
	m_callStackGrid->addColumn(new ui::custom::GridColumn(L"Script", 200));

	tabDebugger->addPage(tabPageCallStack);

	Ref< ui::TabPage > tabPageLocals = new ui::TabPage();
	tabPageLocals->create(tabDebugger, L"Locals", new ui::TableLayout(L"100%", L"100%", 0, 0));

	m_localsGrid = new ui::custom::GridView();
	m_localsGrid->create(tabPageLocals, ui::WsDoubleBuffer | ui::custom::GridView::WsColumnHeader);
	m_localsGrid->addColumn(new ui::custom::GridColumn(L"Name", 200));
	m_localsGrid->addColumn(new ui::custom::GridColumn(L"Value", 300));

	tabDebugger->addPage(tabPageLocals);

	tabDebugger->setActivePage(tabPageCallStack);
	tabDebugger->update();

	return true;
}

void ScriptDebuggerView::destroy()
{
	ui::Container::destroy();
}

bool ScriptDebuggerView::handleCommand(const ui::Command& command)
{
	if (command == L"Script.Editor.Continue")
		m_scriptDebugger->actionContinue();
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

void ScriptDebuggerView::addBreakPointEventHandler(ui::EventHandler* eventHandler)
{
	addEventHandler(EiBreakPoint, eventHandler);
}

Ref< ui::custom::GridRow > ScriptDebuggerView::createVariableRow(const script::Local* local)
{
	Ref< ui::custom::GridRow > row = new ui::custom::GridRow(0);
	row->add(new ui::custom::GridItem(local->getName()));
	if (const script::LocalComposite* composite = dynamic_type_cast< const script::LocalComposite* >(local))
	{
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

void ScriptDebuggerView::breakpointReached(IScriptDebugger* scriptDebugger, const CallStack& callStack)
{
	const script::CallStack::Frame& currentFrame = callStack.getCurrentFrame();

	m_callStackGrid->removeAllRows();

	const std::list< script::CallStack::Frame >& frames = callStack.getFrames();
	for (std::list< script::CallStack::Frame >::const_iterator i = frames.begin(); i != frames.end(); ++i)
	{
		Ref< ui::custom::GridRow > row = new ui::custom::GridRow(0);
		row->add(new ui::custom::GridItem(i->functionName));
		row->add(new ui::custom::GridItem(toString(i->line + 1)));
		row->add(new ui::custom::GridItem(i->scriptName));
		m_callStackGrid->addRow(row);
	}

	m_localsGrid->removeAllRows();

	const RefArray< script::Local >& locals = currentFrame.locals;
	for (RefArray< script::Local >::const_iterator i = locals.begin(); i != locals.end(); ++i)
	{
		Ref< ui::custom::GridRow > row = createVariableRow(*i);
		if (row)
			m_localsGrid->addRow(row);
	}

	ui::Event eventBreakPoint(this, const_cast< CallStack* >(&callStack));
	raiseEvent(EiBreakPoint, &eventBreakPoint);
}

void ScriptDebuggerView::eventDebuggerToolClick(ui::Event* event)
{
	const ui::CommandEvent* cmdEvent = checked_type_cast< const ui::CommandEvent* >(event);
	handleCommand(cmdEvent->getCommand());
}

	}
}
