#ifndef traktor_script_ScriptDebuggerLua_H
#define traktor_script_ScriptDebuggerLua_H

#include <list>
#include <map>
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

	virtual bool setBreakpoint(const Guid& scriptId, uint32_t lineNumber);

	virtual bool removeBreakpoint(const Guid& scriptId, uint32_t lineNumber);

	virtual void addListener(IListener* listener);

	virtual void removeListener(IListener* listener);

private:
	lua_State* m_luaState;
	static std::list< ScriptDebuggerLua* > ms_instances;
	std::map< uint32_t, std::set< Guid > > m_breakpoints;
	std::set< IListener* > m_listeners;

	void analyzeState(lua_State* L, lua_Debug* ar);

	static void hookCallback(lua_State* L, lua_Debug* ar);
};

	}
}

#endif	// traktor_script_ScriptDebuggerLua_H
