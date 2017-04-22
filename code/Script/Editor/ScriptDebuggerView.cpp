#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPage.h"
#include "I18N/Text.h"
#include "Script/StackFrame.h"
#include "Script/Variable.h"
#include "Script/Value.h"
#include "Script/ValueObject.h"
#include "Script/Editor/ScriptDebuggerView.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/MenuItem.h"
#include "Ui/PopupMenu.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridRowStateChangeEvent.h"
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
	m_localsGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_EDITOR_DEBUG_LOCAL_NAME"), ui::scaleBySystemDPI(180)));
	m_localsGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_EDITOR_DEBUG_LOCAL_VALUE"), ui::scaleBySystemDPI(150)));
	m_localsGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCRIPT_EDITOR_DEBUG_LOCAL_TYPE"), ui::scaleBySystemDPI(150)));
	m_localsGrid->setEnable(false);
	m_localsGrid->addEventHandler< ui::custom::GridRowStateChangeEvent >(this, &ScriptDebuggerView::eventLocalsGridStateChange);
	m_localsGrid->addEventHandler< ui::MouseButtonDownEvent >(this, &ScriptDebuggerView::eventLocalsGridButtonDown);

	m_localsPopup = new ui::PopupMenu();
	m_localsPopup->create();
	m_localsPopup->add(new ui::MenuItem(ui::Command(L"Script.Editor.CopyLocalValue"), i18n::Text(L"SCRIPT_EDITOR_DEBUG_COPY_LOCAL_VALUE")));

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
	safeDestroy(m_localsPopup);
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

Ref< ui::custom::GridRow > ScriptDebuggerView::createVariableRow(const Variable* local)
{
	Ref< ui::custom::GridRow > row = new ui::custom::GridRow(0);
	row->add(new ui::custom::GridItem(local->getName()));

	if (const script::Value* value = dynamic_type_cast< const script::Value* >(local->getValue()))
	{
		row->add(new ui::custom::GridItem(value->getLiteral()));
		row->add(new ui::custom::GridItem(local->getTypeName()));
	}
	else if (const script::ValueObject* valueObject = dynamic_type_cast< const script::ValueObject* >(local->getValue()))
	{
		row->add(new ui::custom::GridItem(valueObject->getValueOf()));
		row->add(new ui::custom::GridItem(local->getTypeName()));

		Ref< ui::custom::GridRow > objectRow = new ui::custom::GridRow(0);
		objectRow->setData(L"OBJECT_REFERENCE", const_cast< script::ValueObject* >(valueObject));
		row->addChild(objectRow);
	}

	return row;
}

void ScriptDebuggerView::updateLocals(int32_t depth)
{
	m_localsGrid->removeAllRows();
	if (depth >= 0 && depth < m_stackFrames.size())
	{
		RefArray< Variable > locals;
		if (m_scriptDebugger->captureLocals(depth, locals))
		{
			for (RefArray< Variable >::const_iterator j = locals.begin(); j != locals.end(); ++j)
			{
				Ref< ui::custom::GridRow > row = createVariableRow(*j);
				if (row)
					m_localsGrid->addRow(row);
			}
		}
	}
	m_localsGrid->update();
}

void ScriptDebuggerView::debugeeStateChange(IScriptDebugger* scriptDebugger)
{
	if (!scriptDebugger->isRunning())
	{
		m_stackFrames.resize(0);
		for (uint32_t depth = 0; ; ++depth)
		{
			Ref< StackFrame > sf;
			if (!scriptDebugger->captureStackFrame(depth, sf))
				break;

			T_FATAL_ASSERT (sf);
			m_stackFrames.push_back(sf);
		}

		m_callStackGrid->removeAllRows();

		bool autoOpenDebuggedScript = m_editor->getSettings()->getProperty< bool >(L"Editor.AutoOpenDebuggedScript", true);

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

			// Open debugged script and issue a "goto line" to scroll script editor to debugged line.
			if (autoOpenDebuggedScript && scriptInstance && i == m_stackFrames.begin())
			{
				m_editor->openEditor(scriptInstance);

				editor::IEditorPage* activeEditorPage = m_editor->getActiveEditorPage();
				if (activeEditorPage)
					activeEditorPage->handleCommand(ui::Command((*i)->getLine(), L"Script.Editor.GotoLine"));
			}
		}

		updateLocals(0);

		m_callStackGrid->setEnable(true);
		m_callStackGrid->update();
		m_localsGrid->setEnable(true);
		m_localsGrid->update();
	}
	else
	{
		m_callStackGrid->setEnable(false);
		m_callStackGrid->update();
		m_localsGrid->setEnable(false);
		m_localsGrid->update();
	}
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
				activeEditorPage->handleCommand(ui::Command(line, L"Script.Editor.GotoLine"));
		}

		updateLocals(depth);
	}
	else
		updateLocals(0);
}

void ScriptDebuggerView::eventLocalsGridStateChange(ui::custom::GridRowStateChangeEvent* event)
{
	Ref< ui::custom::GridRow > row = event->getRow();

	const RefArray< ui::custom::GridRow >& children = row->getChildren();
	if (children.size() == 1)
	{
		Ref< script::ValueObject > valueObject = children[0]->getData< script::ValueObject >(L"OBJECT_REFERENCE");
		if (valueObject)
		{
			row->removeAllChildren();

			RefArray< Variable > members;
			if (m_scriptDebugger->captureObject(valueObject->getObjectRef(), members))
			{
				for (RefArray< Variable >::const_iterator j = members.begin(); j != members.end(); ++j)
				{
					Ref< ui::custom::GridRow > childRow = createVariableRow(*j);
					if (childRow)
						row->addChild(childRow);
				}
			}
		}
	}

	m_localsGrid->update();
}

void ScriptDebuggerView::eventLocalsGridButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() != ui::MbtRight)
		return;

	Ref< ui::MenuItem > selected = m_localsPopup->show(m_localsGrid, event->getPosition());
	if (!selected)
		return;

	if (selected->getCommand() == L"Script.Editor.CopyLocalValue")
	{
		ui::custom::GridRow* selectedRow = m_localsGrid->getSelectedRow();
		if (selectedRow)
		{
			std::wstring value = selectedRow->get(1)->getText();
			ui::Application::getInstance()->getClipboard()->setText(value);
		}
	}
}

	}
}
