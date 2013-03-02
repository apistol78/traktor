#ifndef traktor_script_ScriptDebuggerView_H
#define traktor_script_ScriptDebuggerView_H

#include "Script/IScriptDebugger.h"
#include "Ui/Container.h"

namespace traktor
{
	namespace ui
	{

class Command;

		namespace custom
		{

class GridRow;
class GridView;
class ToolBar;

		}
	}

	namespace script
	{

class Local;

class ScriptDebuggerView
:	public ui::Container
,	public IScriptDebugger::IListener
{
	T_RTTI_CLASS;

public:
	ScriptDebuggerView(IScriptDebugger* scriptDebugger);

	virtual ~ScriptDebuggerView();

	bool create(ui::Widget* parent);

	void destroy();

	bool handleCommand(const ui::Command& command);

	void addBreakPointEventHandler(ui::EventHandler* eventHandler);

private:
	Ref< IScriptDebugger > m_scriptDebugger;
	Ref< ui::custom::ToolBar > m_debuggerTools;
	Ref< ui::custom::GridView > m_callStackGrid;
	Ref< ui::custom::GridView > m_localsGrid;

	Ref< ui::custom::GridRow > createVariableRow(const script::Local* local);

	/*! \name IScriptDebugger::IListener */
	/*! \{ */

	virtual void breakpointReached(IScriptDebugger* scriptDebugger, const CallStack& callStack);

	/*! \} */

	void eventDebuggerToolClick(ui::Event* event);
};

	}
}

#endif	// traktor_script_ScriptDebuggerView_H
