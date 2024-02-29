/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Script/IScriptDebugger.h"
#include "Ui/Container.h"

namespace traktor::editor
{

class IEditor;

}

namespace traktor::ui
{

class Command;
class GridRow;
class GridRowStateChangeEvent;
class GridView;
class Menu;
class ToolBar;
class ToolBarButtonClickEvent;

}

namespace traktor::script
{

class Variable;

class ScriptDebuggerView
:	public ui::Container
,	public IScriptDebugger::IListener
{
	T_RTTI_CLASS;

public:
	explicit ScriptDebuggerView(editor::IEditor* editor, IScriptDebugger* scriptDebugger);

	virtual ~ScriptDebuggerView();

	bool create(ui::Widget* parent);

	virtual void destroy() override final;

	bool handleCommand(const ui::Command& command);

private:
	editor::IEditor* m_editor;
	Ref< IScriptDebugger > m_scriptDebugger;
	Ref< ui::ToolBar > m_debuggerTools;
	Ref< ui::GridView > m_callStackGrid;
	Ref< ui::GridView > m_localsGrid;
	Ref< ui::Menu > m_localsPopup;
	RefArray< StackFrame > m_stackFrames;
	int32_t m_activeFrame;

	Ref< ui::GridRow > createVariableRow(const Variable* local);

	void updateLocals(int32_t depth);

	/*! \name IScriptDebugger::IListener */
	/*! \{ */

	virtual void debugeeStateChange(IScriptDebugger* scriptDebugger) override final;

	/*! \} */

	void eventDebuggerToolClick(ui::ToolBarButtonClickEvent* event);

	void eventCallStackGridDoubleClick(ui::MouseDoubleClickEvent* event);

	void eventLocalsGridStateChange(ui::GridRowStateChangeEvent* event);

	void eventLocalsGridButtonDown(ui::MouseButtonDownEvent* event);
};

}
