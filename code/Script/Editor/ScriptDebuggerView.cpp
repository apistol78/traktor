/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Splitter.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridRowStateChangeEvent.h"
#include "Ui/GridView/GridView.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"

namespace traktor::script
{
	namespace
	{

struct VariablePred
{
	bool operator () (const Variable* vl, const Variable* vr) const
	{
		return compareIgnoreCase(vl->getName(), vr->getName()) <= 0;
	}
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptDebuggerView", ScriptDebuggerView, ui::Container)

ScriptDebuggerView::ScriptDebuggerView(editor::IEditor* editor, IScriptDebugger* scriptDebugger)
:	m_editor(editor)
,	m_scriptDebugger(scriptDebugger)
{
}

ScriptDebuggerView::~ScriptDebuggerView()
{
	T_ASSERT(!m_scriptDebugger);
}

bool ScriptDebuggerView::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%,*", 0_ut, 0_ut)))
		return false;

	m_debuggerTools = new ui::ToolBar();
	if (!m_debuggerTools->create(this))
		return false;

	m_debuggerTools->addImage(new ui::StyleBitmap(L"Script.DebugContinue"));
	m_debuggerTools->addImage(new ui::StyleBitmap(L"Script.DebugStepInto"));
	m_debuggerTools->addImage(new ui::StyleBitmap(L"Script.DebugStepOver"));
	m_debuggerTools->addImage(new ui::StyleBitmap(L"Script.DebugStop"));
	m_debuggerTools->addItem(new ui::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_CONTINUE"), 0, ui::Command(L"Script.Editor.Continue")));
	m_debuggerTools->addItem(new ui::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_BREAK"), 3, ui::Command(L"Script.Editor.Break")));
	m_debuggerTools->addItem(new ui::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_STEP_INTO"), 1, ui::Command(L"Script.Editor.StepInto")));
	m_debuggerTools->addItem(new ui::ToolBarButton(i18n::Text(L"SCRIPT_EDITOR_STEP_OVER"), 2, ui::Command(L"Script.Editor.StepOver")));
	m_debuggerTools->addEventHandler< ui::ToolBarButtonClickEvent >(this, &ScriptDebuggerView::eventDebuggerToolClick);

	Ref< ui::Splitter > splitter = new ui::Splitter();
	splitter->create(this, true, 50_ut, true);

	m_callStackGrid = new ui::GridView();
	m_callStackGrid->create(splitter, ui::WsDoubleBuffer | ui::GridView::WsColumnHeader);
	m_callStackGrid->addColumn(new ui::GridColumn(i18n::Text(L"SCRIPT_EDITOR_DEBUG_FUNCTION"), 180_ut));
	m_callStackGrid->addColumn(new ui::GridColumn(i18n::Text(L"SCRIPT_EDITOR_DEBUG_LINE"), 100_ut));
	m_callStackGrid->addColumn(new ui::GridColumn(i18n::Text(L"SCRIPT_EDITOR_DEBUG_SCRIPT"), 200_ut));
	m_callStackGrid->setEnable(false);
	m_callStackGrid->addEventHandler< ui::MouseDoubleClickEvent >(this, &ScriptDebuggerView::eventCallStackGridDoubleClick);

	m_localsGrid = new ui::GridView();
	m_localsGrid->create(splitter, ui::WsDoubleBuffer | ui::GridView::WsColumnHeader);
	m_localsGrid->addColumn(new ui::GridColumn(i18n::Text(L"SCRIPT_EDITOR_DEBUG_LOCAL_NAME"), 180_ut));
	m_localsGrid->addColumn(new ui::GridColumn(i18n::Text(L"SCRIPT_EDITOR_DEBUG_LOCAL_VALUE"), 150_ut));
	m_localsGrid->addColumn(new ui::GridColumn(i18n::Text(L"SCRIPT_EDITOR_DEBUG_LOCAL_TYPE"), 150_ut));
	m_localsGrid->setEnable(false);
	m_localsGrid->addEventHandler< ui::GridRowStateChangeEvent >(this, &ScriptDebuggerView::eventLocalsGridStateChange);
	m_localsGrid->addEventHandler< ui::MouseButtonDownEvent >(this, &ScriptDebuggerView::eventLocalsGridButtonDown);

	m_localsPopup = new ui::Menu();
	m_localsPopup->add(new ui::MenuItem(ui::Command(L"Script.Editor.CopyLocalValue"), i18n::Text(L"SCRIPT_EDITOR_DEBUG_COPY_LOCAL_VALUE")));

	m_scriptDebugger->addListener(this);
	return true;
}

void ScriptDebuggerView::destroy()
{
	if (m_scriptDebugger)
	{
		m_scriptDebugger->removeListener(this);
		m_scriptDebugger = nullptr;
	}
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

Ref< ui::GridRow > ScriptDebuggerView::createVariableRow(const Variable* local)
{
	Ref< ui::GridRow > row = new ui::GridRow(0);
	row->add(local->getName());

	if (const script::Value* value = dynamic_type_cast< const script::Value* >(local->getValue()))
	{
		row->add(value->getLiteral());
		row->add(local->getTypeName());
	}
	else if (const script::ValueObject* valueObject = dynamic_type_cast< const script::ValueObject* >(local->getValue()))
	{
		row->add(valueObject->getValueOf());
		row->add(local->getTypeName());

		Ref< ui::GridRow > objectRow = new ui::GridRow(0);
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
			locals.sort(VariablePred());
			for (auto local : locals)
			{
				Ref< ui::GridRow > row = createVariableRow(local);
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
		// Capture all stack frames.
		m_stackFrames.resize(0);
		for (uint32_t depth = 0; ; ++depth)
		{
			Ref< StackFrame > sf;
			if (!scriptDebugger->captureStackFrame(depth, sf))
				break;
			T_FATAL_ASSERT (sf);
			m_stackFrames.push_back(sf);
		}

		const bool autoOpenDebuggedScript = m_editor->getSettings()->getProperty< bool >(L"Editor.AutoOpenDebuggedScript", true);

		Ref< ui::HierarchicalState > state = m_callStackGrid->captureState();
		m_callStackGrid->removeAllRows();

		int32_t depth = 0;
		for (auto stackFrame : m_stackFrames)
		{
			Ref< db::Instance > scriptInstance = m_editor->getSourceDatabase()->getInstance(stackFrame->getScriptId());

			Ref< ui::GridRow > row = new ui::GridRow(0);
			row->add(stackFrame->getFunctionName());
			row->add(toString(stackFrame->getLine() + 1));
			row->add(scriptInstance ? scriptInstance->getName() : L"(Unknown script)");
			row->setData(L"SCRIPT_ID", new PropertyString(stackFrame->getScriptId().format()));
			row->setData(L"SCRIPT_LINE", new PropertyInteger(stackFrame->getLine()));
			row->setData(L"FRAME_DEPTH", new PropertyInteger(depth++));
			m_callStackGrid->addRow(row);

			// Open debugged script and issue a "goto line" to scroll script editor to debugged line.
			if (autoOpenDebuggedScript && scriptInstance && stackFrame == m_stackFrames.front())
			{
				m_editor->openEditor(scriptInstance);

				editor::IEditorPage* activeEditorPage = m_editor->getActiveEditorPage();
				if (activeEditorPage)
					activeEditorPage->handleCommand(ui::Command(stackFrame->getLine(), L"Script.Editor.GotoLine"));
			}
		}

		updateLocals(0);

		m_callStackGrid->setEnable(true);
		m_callStackGrid->applyState(state);
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

void ScriptDebuggerView::eventDebuggerToolClick(ui::ToolBarButtonClickEvent* event)
{
	handleCommand(event->getCommand());
}

void ScriptDebuggerView::eventCallStackGridDoubleClick(ui::MouseDoubleClickEvent* event)
{
	if (!m_callStackGrid->isEnable(true))
		return;

	ui::GridRow* selectedRow = m_callStackGrid->getSelectedRow();
	if (selectedRow)
	{
		const Guid scriptId = Guid(*(selectedRow->getData< PropertyString >(L"SCRIPT_ID")));
		const int32_t line = *(selectedRow->getData< PropertyInteger >(L"SCRIPT_LINE"));
		const int32_t depth = *(selectedRow->getData< PropertyInteger >(L"FRAME_DEPTH"));

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

void ScriptDebuggerView::eventLocalsGridStateChange(ui::GridRowStateChangeEvent* event)
{
	Ref< ui::GridRow > row = event->getRow();

	const RefArray< ui::GridRow >& children = row->getChildren();
	if (children.size() == 1)
	{
		Ref< script::ValueObject > valueObject = children[0]->getData< script::ValueObject >(L"OBJECT_REFERENCE");
		if (valueObject)
		{
			row->removeAllChildren();

			RefArray< Variable > members;
			if (m_scriptDebugger->captureObject(valueObject->getObjectRef(), members))
			{
				members.sort(VariablePred());
				for (auto member : members)
				{
					Ref< ui::GridRow > childRow = createVariableRow(member);
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

	const ui::MenuItem* selected = m_localsPopup->showModal(m_localsGrid, event->getPosition());
	if (!selected)
		return;

	if (selected->getCommand() == L"Script.Editor.CopyLocalValue")
	{
		ui::GridRow* selectedRow = m_localsGrid->getSelectedRow();
		if (selectedRow)
		{
			const std::wstring value = selectedRow->get(1)->getText();
			ui::Application::getInstance()->getClipboard()->setText(value);
		}
	}
}

}
