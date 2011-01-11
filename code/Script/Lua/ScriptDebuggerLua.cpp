#include "Core/Guid.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
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
			lua_getinfo(L, "S", ar);

			Guid scriptId(mbstows(ar->source));
			if (scriptId.isValid() && !scriptId.isNull())
			{
				if (i->second.find(scriptId) != i->second.end())
				{
					log::info << L"BREAKPOINT HIT" << Endl;
					log::info << L"AT: " << mbstows(ar->source) << L"; line " << (ar->currentline - 1) << Endl;

					const char* localName;
					for (int n = 1; (localName = lua_getlocal(L, ar, n)) != 0; ++n)
					{
						if (*localName != '(')
						{
							const char* localValue = lua_tostring(L, 1);
							if (localValue)
								log::info << L"\t" << n << L". \"" << mbstows(localName) << L"\" = " << mbstows(localValue) << Endl;
							else
								log::info << L"\t" << n << L". \"" << mbstows(localName) << L"\" = nil" << Endl;
						}
						lua_pop(L, 1);
					}

					for (std::set< IListener* >::const_iterator j = m_listeners.begin(); j != m_listeners.end(); ++j)
						(*j)->breakpointReached(this, scriptId, ar->currentline - 1);
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
