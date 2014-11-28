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

ScriptProfilerLua::ScriptProfilerLua(ScriptManagerLua* scriptManager, lua_State* luaState)
:	m_scriptManager(scriptManager)
,	m_luaState(luaState)
{
	m_timer.start();
}

ScriptProfilerLua::~ScriptProfilerLua()
{
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
	if (ar->event == LUA_HOOKLINE)
		return;

	ScriptContextLua* currentContext = m_scriptManager->m_lockContext;
	if (!currentContext)
		return;

	lua_getinfo(L, "Sln", ar);
	if (!ar->name)
		return;

	double timeStamp = m_timer.getElapsedTime();

#if defined(T_LUA_5_2)
	if (ar->event == LUA_HOOKCALL || ar->event == LUA_HOOKTAILCALL)
#else
	if (ar->event == LUA_HOOKCALL)
#endif
	{
		ProfileStack ps;
		ps.timeStamp = timeStamp;
		ps.childDuration = 0.0;
		m_stack.push_back(ps);
	}
#if defined(T_LUA_5_2)
	else if (ar->event == LUA_HOOKRET)
#else
	else if (ar->event == LUA_HOOKRET || ar->event == LUA_HOOKTAILRET)
#endif
	{
		// Make sure we don't break if hooks are behaving strange.
		if (m_stack.empty())
			return;

		Guid scriptId;
		std::wstring name = mbstows(ar->name ? ar->name : "(Unnamed)");

		if (*ar->what != 'C')
		{
			int32_t currentLine = 0;
			if (ar->linedefined >= 1)
			{
				currentLine = ar->linedefined - 1;
				const source_map_t& map = currentContext->m_map;
				for (source_map_t::const_reverse_iterator i = map.rbegin(); i != map.rend(); ++i)
				{
					if (currentLine >= i->line)
					{
						scriptId = i->id;
						currentLine = currentLine - i->line;
						break;
					}
				}
			}
			name += L":" + toString(currentLine);
		}

		ProfileStack& ps = m_stack.back();

		double inclusiveDuration = timeStamp - ps.timeStamp;
		double exclusiveDuration = inclusiveDuration - ps.childDuration;

		// Notify all listeners about new measurement.
		for (std::set< IListener* >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
			(*i)->callMeasured(scriptId, name, 1, inclusiveDuration, exclusiveDuration);

		m_stack.pop_back();

		// Accumulate child call duration so we can isolate how much time is spent in function and not in calls.
		if (!m_stack.empty())
			m_stack.back().childDuration += inclusiveDuration;
	}
}

	}
}
