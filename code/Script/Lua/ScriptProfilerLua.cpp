#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Script/Lua/ScriptContextLua.h"
#include "Script/Lua/ScriptManagerLua.h"
#include "Script/Lua/ScriptProfilerLua.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptProfilerLua", ScriptProfilerLua, IScriptProfiler)

ScriptProfilerLua* ScriptProfilerLua::ms_instance = 0;

ScriptProfilerLua::ScriptProfilerLua(ScriptManagerLua* scriptManager, lua_State* luaState)
:	m_scriptManager(scriptManager)
,	m_luaState(luaState)
{
	T_ASSERT (!ms_instance);
	ms_instance = this;
	lua_sethook(m_luaState, &ScriptProfilerLua::hookCallback, LUA_MASKCALL | LUA_MASKRET, 0);
	m_timer.start();
}

ScriptProfilerLua::~ScriptProfilerLua()
{
	T_ASSERT (ms_instance == this);
	lua_sethook(m_luaState, &ScriptProfilerLua::hookCallback, 0, 0);
	ms_instance = 0;
}

void ScriptProfilerLua::addListener(IListener* listener)
{
	m_listeners.insert(listener);
}

void ScriptProfilerLua::removeListener(IListener* listener)
{
	m_listeners.erase(listener);
}

void ScriptProfilerLua::hookCallback(lua_State* L, lua_Debug* ar)
{
	T_ASSERT (ms_instance);

	ScriptContextLua* currentContext = ms_instance->m_scriptManager->m_lockContext;
	if (!currentContext)
		return;

	lua_getinfo(L, "Sln", ar);
	if (!ar->name)
		return;

	double timeStamp = ms_instance->m_timer.getElapsedTime();
	std::wstring name = mbstows(ar->name ? ar->name : "(Unnamed)") + L":" + toString(ar->linedefined);

	if (ar->event == LUA_HOOKCALL)
	{
		ProfileStack ps;
		ps.function = name;
		ps.timeStamp = timeStamp;
		ps.childDuration = 0.0;
		ms_instance->m_stack.push_back(ps);
	}
	else if (ar->event == LUA_HOOKRET || ar->event == LUA_HOOKTAILRET)
	{
		// Make sure we don't break if hooks are behaving strange.
		if (ms_instance->m_stack.empty())
			return;

		std::wstring currentName = name;
		int32_t currentLine = 0;

		if (ar->linedefined >= 1)
		{
			currentLine = ar->linedefined - 1;

			const source_map_t& map = currentContext->m_map;
			for (source_map_t::const_reverse_iterator i = map.rbegin(); i != map.rend(); ++i)
			{
				if (currentLine >= i->line)
				{
					currentName = i->name + L" " + name;
					currentLine = currentLine - i->line;
					break;
				}
			}
		}

		ProfileStack& ps = ms_instance->m_stack.back();

		double inclusiveDuration = timeStamp - ps.timeStamp;
		double exclusiveDuration = inclusiveDuration - ps.childDuration;

		// Notify all listeners about new measurement.
		for (std::set< IListener* >::const_iterator i = ms_instance->m_listeners.begin(); i != ms_instance->m_listeners.end(); ++i)
			(*i)->callMeasured(currentName, ps.timeStamp, inclusiveDuration, exclusiveDuration);

		ms_instance->m_stack.pop_back();

		// Accumulate child call duration so we can isolate how much time is spent in function and not in calls.
		if (!ms_instance->m_stack.empty())
			ms_instance->m_stack.back().childDuration += inclusiveDuration;
	}
}

	}
}
