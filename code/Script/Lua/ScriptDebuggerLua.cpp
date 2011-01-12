#include "Core/Guid.h"
#include "Core/Misc/TString.h"
#include "Script/CallStack.h"
#include "Script/Lua/ScriptDebuggerLua.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptDebuggerLua", ScriptDebuggerLua, IScriptDebugger)

std::list< ScriptDebuggerLua* > ScriptDebuggerLua::ms_instances;

ScriptDebuggerLua::ScriptDebuggerLua(ScriptManagerLua* scriptManager, lua_State* luaState)
:	m_luaState(luaState)
{
	if (ms_instances.empty())
		lua_sethook(m_luaState, &ScriptDebuggerLua::hookCallback, LUA_MASKLINE, 0);
	ms_instances.push_back(this);
}

ScriptDebuggerLua::~ScriptDebuggerLua()
{
	ms_instances.remove(this);
	if (ms_instances.empty())
		lua_sethook(m_luaState, &ScriptDebuggerLua::hookCallback, 0, 0);
}

bool ScriptDebuggerLua::setBreakpoint(const Guid& scriptId, uint32_t lineNumber)
{
	m_breakpoints[lineNumber].insert(scriptId);
	return true;
}

bool ScriptDebuggerLua::removeBreakpoint(const Guid& scriptId, uint32_t lineNumber)
{
	m_breakpoints[lineNumber].erase(scriptId);
	return true;
}

void ScriptDebuggerLua::addListener(IListener* listener)
{
	m_listeners.insert(listener);
}

void ScriptDebuggerLua::removeListener(IListener* listener)
{
	m_listeners.erase(listener);
}

void ScriptDebuggerLua::analyzeState(lua_State* L, lua_Debug* ar)
{
	std::map< uint32_t, std::set< Guid > >::const_iterator i = m_breakpoints.find(ar->currentline - 1);
	if (i != m_breakpoints.end())
	{
		if (lua_getstack(L, 0, ar))
		{
			lua_getinfo(L, "Sn", ar);

			Guid scriptId(mbstows(ar->source));
			if (scriptId.isValid() && !scriptId.isNull())
			{
				if (i->second.find(scriptId) != i->second.end())
				{
					CallStack cs;

					CallStack::Frame f;
					f.name = ar->name ? mbstows(ar->name) : L"";
					f.scriptId = scriptId;
					f.lineNumber = ar->currentline - 1;

					const char* localName;
					for (int n = 1; (localName = lua_getlocal(L, ar, n)) != 0; ++n)
					{
						if (*localName != '(')
						{
							CallStack::Local l;
							l.name = mbstows(localName);

							const char* localValue = lua_tostring(L, 1);
							if (localValue)
								l.value = mbstows(localValue);

							f.locals.push_back(l);
						}
						lua_pop(L, 1);
					}

					cs.pushFrame(f);

					for (std::set< IListener* >::const_iterator j = m_listeners.begin(); j != m_listeners.end(); ++j)
						(*j)->breakpointReached(this, cs);
				}
			}
		}
	}
}

void ScriptDebuggerLua::hookCallback(lua_State* L, lua_Debug* ar)
{
	for (std::list< ScriptDebuggerLua* >::const_iterator i = ms_instances.begin(); i != ms_instances.end(); ++i)
		(*i)->analyzeState(L, ar);
}

	}
}
