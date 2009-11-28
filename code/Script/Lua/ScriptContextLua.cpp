#include <csetjmp>
#include "Script/Lua/ScriptContextLua.h"
#include "Script/IScriptClass.h"
#include "Core/Misc/TString.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Log/Log.h"

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
		namespace
		{

#if defined(_DEBUG)
class CheckStack
{
public:
	CheckStack(lua_State* luaState, int32_t expectedOffset = 0)
	:	m_luaState(luaState)
	,	m_top(lua_gettop(luaState) + expectedOffset)
	{
	}

	~CheckStack()
	{
		int32_t top = lua_gettop(m_luaState);
		T_ASSERT (m_top == top);
	}

private:
	lua_State* m_luaState;
	int32_t m_top;
};
#	define CHECK_LUA_STACK(state, expectedOffset) CheckStack cs(state, expectedOffset);
#else
#	define CHECK_LUA_STACK(state, expectedOffset)
#endif

std::jmp_buf s_jb;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptContextLua", ScriptContextLua, IScriptContext)

ScriptContextLua::ScriptContextLua(const RefArray< IScriptClass >& registeredClasses)
{
	m_luaState = lua_open();

	lua_atpanic(m_luaState, luaPanic);

	luaopen_base(m_luaState);
	luaopen_table(m_luaState);
	luaopen_string(m_luaState);
	luaopen_math(m_luaState);

	for (RefArray< IScriptClass >::const_iterator i = registeredClasses.begin(); i != registeredClasses.end(); ++i)
		registerClass(*i);
}

ScriptContextLua::~ScriptContextLua()
{
	lua_close(m_luaState);
}

void ScriptContextLua::setGlobal(const std::wstring& globalName, const Any& globalValue)
{
	CHECK_LUA_STACK(m_luaState, 0);
	pushAny(globalValue);
	lua_setglobal(m_luaState, wstombs(globalName).c_str());
}

Any ScriptContextLua::getGlobal(const std::wstring& globalName)
{
	CHECK_LUA_STACK(m_luaState, 0);
	lua_getglobal(m_luaState, wstombs(globalName).c_str());
	return toAny(-1);
}

bool ScriptContextLua::executeScript(const std::wstring& script, bool compileOnly, IErrorCallback* errorCallback)
{
	CHECK_LUA_STACK(m_luaState, 0);

	int32_t result = luaL_loadstring(m_luaState, wstombs(script).c_str());
	if (result == LUA_ERRSYNTAX)
	{
		if (errorCallback)
		{
			std::wstring message = mbstows(lua_tostring(m_luaState, -1));
			std::vector< std::wstring > parts;
			Split< std::wstring >::any(message, L":", parts);
			T_ASSERT (parts.size() == 3);
			errorCallback->syntaxError(parseString< int32_t >(parts[1]), parts[2]);
		}
		else
			log::error << L"LUA syntax error; " << mbstows(lua_tostring(m_luaState, -1)) << Endl;

		lua_pop(m_luaState, 1);
		return false;
	}
	else if (result == LUA_ERRMEM)
	{
		if (errorCallback)
			errorCallback->otherError(L"Out of memory in LUA runtime; " + mbstows(lua_tostring(m_luaState, -1)));
		else
			log::debug << L"Out of memory in LUA runtime; " << mbstows(lua_tostring(m_luaState, -1)) << Endl;
		lua_pop(m_luaState, 1);
		return false;
	}

	if (!compileOnly)
		lua_pcall(m_luaState, 0, LUA_MULTRET, 0);
	else
		lua_pop(m_luaState, 1);

	return true;
}

bool ScriptContextLua::haveFunction(const std::wstring& functionName) const
{
	CHECK_LUA_STACK(m_luaState, 0);

	lua_getglobal(m_luaState, wstombs(functionName).c_str());
	if (!lua_isnil(m_luaState, -1)) 
	{
		lua_pop(m_luaState, 1);
		return true;
	} 
	else 
	{
		lua_pop(m_luaState, 1);
		return false;
	}
}

Any ScriptContextLua::executeFunction(const std::wstring& functionName, const std::vector< Any >& arguments)
{
	CHECK_LUA_STACK(m_luaState, 0);

	Any returnValue;

#if !defined(_WIN32) && !defined(__APPLE__)
	if (std::setjmp(s_jb) == 0)
#else
	if (setjmp(s_jb) == 0)
#endif
	{
		lua_getglobal(m_luaState, wstombs(functionName).c_str());
		if (!lua_isnil(m_luaState, -1))
		{
			for (std::vector< Any >::const_iterator i = arguments.begin(); i != arguments.end(); ++i)
				pushAny(*i);
			lua_call(m_luaState, int32_t(arguments.size()), 1);
			returnValue = toAny(-1);
		}
		lua_pop(m_luaState, 1);
		lua_gc(m_luaState, LUA_GCCOLLECT, 0);
	}

	return returnValue;
}

Any ScriptContextLua::executeMethod(Object* self, const std::wstring& methodName, const std::vector< Any >& arguments)
{
	CHECK_LUA_STACK(m_luaState, 0);

	Any returnValue;

#if !defined(_WIN32) && !defined(__APPLE__)
	if (std::setjmp(s_jb) == 0)
#else
	if (setjmp(s_jb) == 0)
#endif
	{
		lua_getglobal(m_luaState, wstombs(methodName).c_str());
		if (!lua_isnil(m_luaState, -1))
		{
			pushAny(Any(self));
			for (std::vector< Any >::const_iterator i = arguments.begin(); i != arguments.end(); ++i)
				pushAny(*i);
			lua_call(m_luaState, int32_t(arguments.size() + 1), 1);
			returnValue = toAny(-1);
		}
		lua_pop(m_luaState, 1);
	}

	return returnValue;
}

void ScriptContextLua::registerClass(IScriptClass* scriptClass)
{
	CHECK_LUA_STACK(m_luaState, 0);

	RegisteredClass rc;

	rc.scriptClass = scriptClass;

	lua_newtable(m_luaState);
	rc.metaTableRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);

	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, rc.metaTableRef);

	lua_pushvalue(m_luaState, -1);
	lua_setfield(m_luaState, -2, "__index");

	lua_pushlightuserdata(m_luaState, (void*)scriptClass);
	lua_setfield(m_luaState, -2, "__class");

	uint32_t methodCount = scriptClass->getMethodCount();
	for (uint32_t i = 0; i < methodCount; ++i)
	{
		std::wstring methodName = scriptClass->getMethodName(i);
		lua_pushinteger(m_luaState, i);
		lua_pushlightuserdata(m_luaState, (void*)this);
		lua_pushlightuserdata(m_luaState, (void*)scriptClass);
		lua_pushcclosure(m_luaState, callMethod, 3);
		lua_setfield(m_luaState, -2, wstombs(methodName).c_str());
	}

	const TypeInfo* superType = scriptClass->getExportType().getSuper();
	T_ASSERT (superType);

	bool exportedAsRoot = true;
	for (std::vector< RegisteredClass >::iterator i = m_classRegistry.begin(); i != m_classRegistry.end(); ++i)
	{
		if (superType == &i->scriptClass->getExportType())
		{
			lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, i->metaTableRef);
			lua_setmetatable(m_luaState, -2);
			exportedAsRoot = false;
			break;
		}
	}

	if (exportedAsRoot)
	{
		lua_newtable(m_luaState);
		
		lua_pushlightuserdata(m_luaState, (void*)this);
		lua_pushlightuserdata(m_luaState, (void*)scriptClass);
		lua_pushcclosure(m_luaState, getUnknownMethod, 2);
		lua_setfield(m_luaState, -2, "__index");

		lua_setmetatable(m_luaState, -2);
	}

	lua_pop(m_luaState, 1);

	m_classRegistry.push_back(rc);
}

void ScriptContextLua::pushAny(const Any& any)
{
	if (any.isBoolean())
		lua_pushboolean(m_luaState, any.getBoolean() ? 1 : 0);
	else if (any.isInteger())
		lua_pushinteger(m_luaState, any.getInteger());
	else if (any.isFloat())
		lua_pushnumber(m_luaState, any.getFloat());
	else if (any.isString())
		lua_pushstring(m_luaState, wstombs(any.getString()).c_str());
	else if (any.isObject())
	{
		if (any.getObject())
		{
			const TypeInfo* objectType = &type_of(any.getObject());
			for (std::vector< RegisteredClass >::iterator i = m_classRegistry.begin(); i != m_classRegistry.end(); ++i)
			{
				if (&i->scriptClass->getExportType() == objectType)
				{
					lua_newtable(m_luaState);

					lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, i->metaTableRef);
					lua_setmetatable(m_luaState, -2);

					void* data = lua_newuserdata(m_luaState, sizeof(void*));		// +1
					*(Ref< Object >**)data = new Ref< Object >(any.getObject());
					lua_newtable(m_luaState);										// +1
					lua_pushcfunction(m_luaState, gcMethod);						// +1
					lua_setfield(m_luaState, -2, "__gc");							// -1
					lua_setmetatable(m_luaState, -2);								// -1

					lua_setfield(m_luaState, -2, "__this");							// -1
					return;
				}
			}
		}
		lua_pushnil(m_luaState);
	}
	else
		lua_pushnil(m_luaState);
}

Any ScriptContextLua::toAny(int32_t index)
{
	if (lua_isnumber(m_luaState, index))
		return Any(float(lua_tonumber(m_luaState, index)));
	if (lua_isboolean(m_luaState, index))
		return Any(bool(lua_toboolean(m_luaState, index) != 0));
	if (lua_isstring(m_luaState, index))
		return Any(mbstows(lua_tostring(m_luaState, index)));
	if (lua_istable(m_luaState, index))
	{
		lua_getfield(m_luaState, index, "__this");
		void* objectRefData = lua_touserdata(m_luaState, -1);
		lua_pop(m_luaState, 1);
		if (objectRefData)
		{
			Ref< Object > object = *(*(Ref< Object >**)(objectRefData));
			return Any(object);
		}
	}
	return Any();
}

int ScriptContextLua::getUnknownMethod(lua_State* luaState)
{
	ScriptContextLua* context = reinterpret_cast< ScriptContextLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT (context);

	if (!lua_istable(luaState, 1))
		return 0;

	const IScriptClass* scriptClass = reinterpret_cast< IScriptClass* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	if (!scriptClass)
		return 0;

	const char* methodName = lua_tostring(luaState, 2);
	T_ASSERT (methodName);

	lua_pushstring(luaState, methodName);
	lua_pushlightuserdata(luaState, (void*)context);
	lua_pushlightuserdata(luaState, (void*)scriptClass);
	lua_pushcclosure(luaState, callUnknownMethod, 3);

	return 1;
}

int ScriptContextLua::callMethod(lua_State* luaState)
{
	CHECK_LUA_STACK(luaState, 1);

	ScriptContextLua* context = reinterpret_cast< ScriptContextLua* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (context);

	if (!lua_istable(luaState, 1))
		return 0;

	// Get script class from function.
	const IScriptClass* scriptClass = reinterpret_cast< IScriptClass* >(lua_touserdata(luaState, lua_upvalueindex(3)));
	if (!scriptClass)
		return 0;

	// Get object pointer.
	lua_getfield(luaState, 1, "__this");
	void* objectRefData = lua_touserdata(luaState, -1);
	lua_pop(luaState, 1);
	if (!objectRefData)
		return 0;

	Ref< Object > object = *(*(Ref< Object >**)(objectRefData));
	if (!object)
		return 0;

	// Get called method id from closure.
	int32_t methodId = (int32_t)lua_tonumber(luaState, lua_upvalueindex(1));

	// Convert arguments.
	int32_t argsCount = lua_gettop(luaState) - 1;
	std::vector< Any > args(argsCount);
	for (int32_t i = 0; i < argsCount; ++i)
		args[i] = context->toAny(2 + i);

	// Invoke native method.
	Any returnValue = scriptClass->invoke(object, methodId, args);
	context->pushAny(returnValue);

	return 1;
}

int ScriptContextLua::callUnknownMethod(lua_State* luaState)
{
	ScriptContextLua* context = reinterpret_cast< ScriptContextLua* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (context);

	if (!lua_istable(luaState, 1))
		return 0;

	const char* methodName = lua_tostring(luaState, lua_upvalueindex(1));
	T_ASSERT (methodName);
	
	const IScriptClass* scriptClass = reinterpret_cast< IScriptClass* >(lua_touserdata(luaState, lua_upvalueindex(3)));
	if (!scriptClass)
		return 0;

	// Get object pointer.
	lua_getfield(luaState, 1, "__this");
	void* objectRefData = lua_touserdata(luaState, -1);
	lua_pop(luaState, 1);
	if (!objectRefData)
		return 0;

	Ref< Object > object = *(*(Ref< Object >**)(objectRefData));
	if (!object)
		return 0;

	// Convert arguments.
	int32_t argsCount = lua_gettop(luaState) - 1;
	std::vector< Any > args(argsCount);
	for (int32_t i = 0; i < argsCount; ++i)
		args[i] = context->toAny(2 + i);

	// Invoke native method.
	Any returnValue = scriptClass->invokeUnknown(object, mbstows(methodName), args);
	context->pushAny(returnValue);

	return 1;
}

int ScriptContextLua::gcMethod(lua_State* luaState)
{
	Ref< Object >* objectRef = *(Ref< Object >**)(lua_touserdata(luaState, 1));
	if (objectRef)
	{
		*objectRef = 0;
		delete objectRef;
	}
	return 0;
}

int ScriptContextLua::luaPanic(lua_State* luaState)
{
	log::error << L"LUA PANIC; Unrecoverable error \"" << mbstows(lua_tostring(luaState, lua_gettop(luaState))) << L"\"" << Endl;
	std::longjmp(s_jb, 1);
	return 0;
}

	}
}
