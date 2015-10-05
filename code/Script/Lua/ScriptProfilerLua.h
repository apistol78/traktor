#ifndef traktor_script_ScriptProfilerLua_H
#define traktor_script_ScriptProfilerLua_H

#include <set>
#include <stack>
#include "Core/Timer/Timer.h"
#include "Script/IScriptProfiler.h"

struct lua_Debug;
struct lua_State;

namespace traktor
{
	namespace script
	{

class ScriptManagerLua;

class ScriptProfilerLua : public IScriptProfiler
{
	T_RTTI_CLASS;

public:
	ScriptProfilerLua(ScriptManagerLua* scriptManager, lua_State* luaState);

	virtual ~ScriptProfilerLua();

	virtual void addListener(IListener* listener) T_OVERRIDE T_FINAL;

	virtual void removeListener(IListener* listener) T_OVERRIDE T_FINAL;

private:
	friend class ScriptManagerLua;

	struct ProfileStack
	{
		double timeStamp;
		double childDuration;
	};

	ScriptManagerLua* m_scriptManager;
	lua_State* m_luaState;
	std::vector< ProfileStack > m_stack;
	std::set< IListener* > m_listeners;
	Timer m_timer;

	void hookCallback(lua_State* L, lua_Debug* ar);
};

	}
}

#endif	// traktor_script_ScriptProfilerLua_H
