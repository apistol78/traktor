#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/ISerializable.h"
#include "Script/Boxes.h"
#include "Script/Delegate.h"
#include "Script/IScriptClass.h"
#include "Script/Lua/ScriptContextLua.h"
#include "Script/Lua/ScriptManagerLua.h"
#include "Script/Lua/ScriptResourceLua.h"

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptManagerLua", 0, ScriptManagerLua, IScriptManager)

ScriptManagerLua::ScriptManagerLua()
{
	registerBoxClasses(this);
	registerDelegateClasses(this);
}

void ScriptManagerLua::registerClass(IScriptClass* scriptClass)
{
	m_registeredClasses.push_back(scriptClass);
}

Ref< IScriptClass > ScriptManagerLua::findScriptClass(const TypeInfo& type) const
{
	Ref< IScriptClass > minScriptClass;
	uint32_t minScriptClassDiff = ~0UL;

	for (RefArray< IScriptClass >::const_iterator i = m_registeredClasses.begin(); i != m_registeredClasses.end(); ++i)
	{
		uint32_t scriptClassDiff = type_difference((*i)->getExportType(), type);
		if (scriptClassDiff < minScriptClassDiff)
		{
			minScriptClass = *i;
			minScriptClassDiff = scriptClassDiff;
		}
	}

	return minScriptClass;
}

Ref< IScriptResource > ScriptManagerLua::compile(const std::wstring& script, bool strip, IErrorCallback* errorCallback) const
{
	lua_State* luaState;

	luaState = lua_open();

	// Compile script into state.
	int32_t result = luaL_loadstring(luaState, wstombs(script).c_str());
	if (result == LUA_ERRSYNTAX)
	{
		if (errorCallback)
		{
			std::wstring message = mbstows(lua_tostring(luaState, -1));
			std::vector< std::wstring > parts;
			Split< std::wstring >::any(message, L":", parts);
			T_ASSERT (parts.size() == 3);
			errorCallback->syntaxError(parseString< int32_t >(parts[1]), parts[2]);
		}
		else
			log::error << L"LUA syntax error; " << mbstows(lua_tostring(luaState, -1)) << Endl;
		lua_pop(luaState, 1);
		lua_close(luaState);
		return 0;
	}
	else if (result == LUA_ERRMEM)
	{
		if (errorCallback)
			errorCallback->otherError(L"Out of memory in LUA runtime; " + mbstows(lua_tostring(luaState, -1)));
		else
			log::error << L"Out of memory in LUA runtime; " << mbstows(lua_tostring(luaState, -1)) << Endl;
		lua_pop(luaState, 1);
		lua_close(luaState);
		return 0;
	}

	lua_close(luaState);

	return new ScriptResourceLua(script);
}

Ref< IScriptContext > ScriptManagerLua::createContext()
{
	return new ScriptContextLua(m_registeredClasses);
}

	}
}
