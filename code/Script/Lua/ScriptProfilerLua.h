#pragma once

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

	virtual void addListener(IListener* listener) override final;

	virtual void removeListener(IListener* listener) override final;

	void notifyCallEnter();

	void notifyCallLeave();

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

