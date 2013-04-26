#ifndef traktor_script_ScriptDebuggerLua_H
#define traktor_script_ScriptDebuggerLua_H

#include <list>
#include <map>
#include "Core/Thread/Semaphore.h"
#include "Script/IScriptDebugger.h"

struct lua_Debug;
struct lua_State;

namespace traktor
{
	namespace script
	{

class ScriptManagerLua;

/*! \brief LUA script debugger
 * \ingroup Script
 */
class ScriptDebuggerLua : public IScriptDebugger
{
	T_RTTI_CLASS;

public:
	ScriptDebuggerLua(ScriptManagerLua* scriptManager, lua_State* luaState);

	virtual ~ScriptDebuggerLua();

	virtual bool setBreakpoint(const Guid& scriptId, int32_t lineNumber);

	virtual bool removeBreakpoint(const Guid& scriptId, int32_t lineNumber);

	virtual bool isRunning() const;

	virtual bool actionBreak();

	virtual bool actionContinue();

	virtual bool actionStepInto();

	virtual bool actionStepOver();

	virtual void addListener(IListener* listener);

	virtual void removeListener(IListener* listener);

private:
	friend class ScriptManagerLua;

	enum State
	{
		StRunning,
		StHalted,
		StBreak,
		StStepInto,
		StStepOver
	};

	ScriptManagerLua* m_scriptManager;
	lua_State* m_luaState;
	static ScriptDebuggerLua* ms_instance;
	Semaphore m_lock;
	std::map< int32_t, std::set< Guid > > m_breakpoints;
	std::set< IListener* > m_listeners;
	Guid m_lastId;
	State m_state;

	void analyzeState(lua_State* L, lua_Debug* ar);

	void captureCallStack(lua_State* L, CallStack& outCallStack);

	static void hookCallback(lua_State* L, lua_Debug* ar);
};

	}
}

#endif	// traktor_script_ScriptDebuggerLua_H
