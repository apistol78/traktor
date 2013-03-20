#include <cstring>
#include "Core/Guid.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Script/Boxes.h"
#include "Script/CallStack.h"
#include "Script/IScriptClass.h"
#include "Script/LocalComposite.h"
#include "Script/LocalSimple.h"
#include "Script/Lua/ScriptContextLua.h"
#include "Script/Lua/ScriptDebuggerLua.h"
#include "Script/Lua/ScriptManagerLua.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

std::wstring describeValue(lua_State* L, int32_t index)
{
	if (lua_isnumber(L, index))
		return toString(lua_tonumber(L, index));
	else if (lua_isboolean(L, index))
		return lua_toboolean(L, index) != 0 ? L"true" : L"false";
	else if (lua_isstring(L, index))
		return mbstows(lua_tostring(L, index));
	else if (lua_isuserdata(L, index))
	{
		Object* object = *reinterpret_cast< Object** >(lua_touserdata(L, index));
		if (object)
		{
			if (const Boxed* box = dynamic_type_cast< const Boxed* >(object))
				return box->toString();
			else
				return std::wstring(L"(") + type_name(object) + std::wstring(L")");
		}
		else
			return L"(null)";
	}
	else
		return L"";
}

Ref< Local > describeLocal(const std::wstring& name, lua_State* L, int32_t index, int depth)
{
	if (lua_istable(L, index))
	{
		RefArray< Local > values;

		lua_pushnil(L);
		while (lua_next(L, index - 1))
		{
			std::wstring name = describeValue(L, -2);
			if (depth < 4)
			{
				Ref< Local > value = describeLocal(name, L, -1, depth + 1);
				if (value)
					values.push_back(value);
			}
			else
				values.push_back(new LocalSimple(name, L"(...)"));
			lua_pop(L, 1);
		}

		return new LocalComposite(
			name,
			values
		);
	}
	else
		return new LocalSimple(
			name,
			describeValue(L, index)
		);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptDebuggerLua", ScriptDebuggerLua, IScriptDebugger)

std::list< ScriptDebuggerLua* > ScriptDebuggerLua::ms_instances;

ScriptDebuggerLua::ScriptDebuggerLua(ScriptManagerLua* scriptManager, lua_State* luaState)
:	m_scriptManager(scriptManager)
,	m_luaState(luaState)
,	m_state(StRunning)
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

bool ScriptDebuggerLua::setBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_breakpoints[lineNumber].insert(scriptId);
	return true;
}

bool ScriptDebuggerLua::removeBreakpoint(const Guid& scriptId, int32_t lineNumber)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_breakpoints[lineNumber].erase(scriptId);
	return true;
}

void ScriptDebuggerLua::addListener(IListener* listener)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_listeners.insert(listener);
}

void ScriptDebuggerLua::removeListener(IListener* listener)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_listeners.erase(listener);
}

bool ScriptDebuggerLua::isRunning() const
{
	return m_state == StRunning;
}

bool ScriptDebuggerLua::actionBreak()
{
	m_state = StBreak;
	return true;
}

bool ScriptDebuggerLua::actionContinue()
{
	m_state = StRunning;
	return true;
}

bool ScriptDebuggerLua::actionStepInto()
{
	m_state = StStepInto;
	return false;
}

bool ScriptDebuggerLua::actionStepOver()
{
	m_state = StStepOver;
	return false;
}

void ScriptDebuggerLua::analyzeState(lua_State* L, lua_Debug* ar)
{
	ScriptContextLua* currentContext = m_scriptManager->m_currentContext;
	if (!currentContext)
		return;

	T_ASSERT (ar->currentline >= 1);

	if (m_state == StRunning)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		Guid currentId;
		std::wstring currentName;
		int32_t currentLine = ar->currentline - 1;

		const source_map_t& map = currentContext->m_map;
		for (source_map_t::const_reverse_iterator i = map.rbegin(); i != map.rend(); ++i)
		{
			if (currentLine >= i->line)
			{
				currentId = i->id;
				currentName = i->name;
				currentLine = currentLine - i->line;
				break;
			}
		}

		std::map< int32_t, std::set< Guid > >::const_iterator i = m_breakpoints.find(currentLine);
		if (
			i != m_breakpoints.end() &&
			i->second.find(currentId) != i->second.end()
		)
		{
			CallStack cs;
			captureCallStack(L, cs);

			m_state = StHalted;
			m_lastId = currentId;

			for (std::set< IListener* >::const_iterator j = m_listeners.begin(); j != m_listeners.end(); ++j)
				(*j)->breakpointReached(this, cs);
		}
	}
	else if (m_state == StBreak)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		Guid currentId;
		int32_t currentLine = ar->currentline - 1;

		const source_map_t& map = currentContext->m_map;
		for (source_map_t::const_reverse_iterator i = map.rbegin(); i != map.rend(); ++i)
		{
			if (currentLine >= i->line)
			{
				currentId = i->id;
				break;
			}
		}

		CallStack cs;
		captureCallStack(L, cs);

		m_state = StHalted;
		m_lastId = currentId;

		for (std::set< IListener* >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
			(*i)->breakpointReached(this, cs);
	}
	else if (m_state == StStepInto)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		Guid currentId;
		int32_t currentLine = ar->currentline - 1;

		const source_map_t& map = currentContext->m_map;
		for (source_map_t::const_reverse_iterator i = map.rbegin(); i != map.rend(); ++i)
		{
			if (currentLine >= i->line)
			{
				currentId = i->id;
				break;
			}
		}

		CallStack cs;
		captureCallStack(L, cs);

		m_state = StHalted;
		m_lastId = currentId;

		for (std::set< IListener* >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
			(*i)->breakpointReached(this, cs);
	}
	else if (m_state == StStepOver)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		Guid currentId;
		int32_t currentLine = ar->currentline - 1;

		const source_map_t& map = currentContext->m_map;
		for (source_map_t::const_reverse_iterator i = map.rbegin(); i != map.rend(); ++i)
		{
			if (currentLine >= i->line)
			{
				currentId = i->id;
				currentLine = currentLine - i->line;
				break;
			}
		}

		if (currentId == m_lastId)
		{
			CallStack cs;
			captureCallStack(L, cs);

			m_state = StHalted;

			for (std::set< IListener* >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
				(*i)->breakpointReached(this, cs);
		}
	}

	if (m_state == StHalted)
	{
		Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
		do
		{
			currentThread->sleep(100);
		}
		while (m_state == StHalted);
	}
}

void ScriptDebuggerLua::captureCallStack(lua_State* L, CallStack& outCallStack)
{
	ScriptContextLua* currentContext = m_scriptManager->m_currentContext;
	T_ASSERT (currentContext);

	lua_Debug ar;
	std::memset(&ar, 0, sizeof(ar));

	for (int level = 0; lua_getstack(L, level, &ar); ++level)
	{
		lua_getinfo(L, "Snlu", &ar);
		T_ASSERT (ar.currentline >= 1);

		Guid currentId;
		std::wstring currentName;
		int32_t currentLine = ar.currentline - 1;

		const source_map_t& map = currentContext->m_map;
		for (source_map_t::const_reverse_iterator i = map.rbegin(); i != map.rend(); ++i)
		{
			if (currentLine >= i->line)
			{
				currentId = i->id;
				currentName = i->name;
				currentLine = currentLine - i->line;
				break;
			}
		}

		CallStack::Frame f;
		f.scriptId = currentId;
		f.scriptName = currentName;
		f.functionName = ar.name ? mbstows(ar.name) : L"(anonymous)";
		f.line = currentLine;

		const char* localName;
		for (int n = 1; (localName = lua_getlocal(L, &ar, n)) != 0; ++n)
		{
			if (*localName != '(')
			{
				Ref< Local > local = describeLocal(mbstows(localName), L, -1, 0);
				if (local)
					f.locals.push_back(local);
			}
			lua_pop(L, 1);
		}

		outCallStack.pushFrame(f);
		std::memset(&ar, 0, sizeof(ar));
	}
}

void ScriptDebuggerLua::hookCallback(lua_State* L, lua_Debug* ar)
{
	for (std::list< ScriptDebuggerLua* >::const_iterator i = ms_instances.begin(); i != ms_instances.end(); ++i)
		(*i)->analyzeState(L, ar);
}

	}
}
