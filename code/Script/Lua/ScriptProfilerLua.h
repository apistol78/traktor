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

	virtual void addListener(IListener* listener);

	virtual void removeListener(IListener* listener);

private:
	struct ProfileStack
	{
		std::wstring function;
		double timeStamp;
		double childDuration;
	};

	static ScriptProfilerLua* ms_instance;
	lua_State* m_luaState;
	std::vector< ProfileStack > m_stack;
	std::set< IListener* > m_listeners;
	Timer m_timer;

	static void hookCallback(lua_State* L, lua_Debug* ar);
};

	}
}

#endif	// traktor_script_ScriptProfilerLua_H
