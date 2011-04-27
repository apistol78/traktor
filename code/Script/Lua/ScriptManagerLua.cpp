#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/ISerializable.h"
#include "Script/Boxes.h"
#include "Script/Delegate.h"
#include "Script/IScriptClass.h"
#include "Script/Lua/ScriptContextLua.h"
#include "Script/Lua/ScriptDebuggerLua.h"
#include "Script/Lua/ScriptManagerLua.h"
#include "Script/Lua/ScriptResourceLua.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

const int32_t c_tableKey_class = -1;
const int32_t c_tableKey_this = -2;

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptManagerLua", 0, ScriptManagerLua, IScriptManager)

ScriptManagerLua::ScriptManagerLua()
:	m_currentContext(0)
,	m_gcMetaRef(0)
{
	m_luaState = lua_newstate(&luaAlloc, 0);

	lua_atpanic(m_luaState, luaPanic);

	luaopen_base(m_luaState);
	luaopen_table(m_luaState);
	luaopen_string(m_luaState);
	luaopen_math(m_luaState);

	lua_register(m_luaState, "print", luaPrint);

	registerBoxClasses(this);
	registerDelegateClasses(this);

	// Create meta table for C++ objects.
	lua_newtable(m_luaState);
	m_gcMetaRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_gcMetaRef);

	lua_pushcfunction(m_luaState, classGcMethod);
	lua_setfield(m_luaState, -2, "__gc");

	lua_pop(m_luaState, 1);

}

ScriptManagerLua::~ScriptManagerLua()
{
	destroy();
}

void ScriptManagerLua::destroy()
{
	if (m_luaState)
	{
		lua_unref(m_luaState, m_gcMetaRef);
		lua_close(m_luaState);
		m_luaState = 0;
	}
}

void ScriptManagerLua::registerClass(IScriptClass* scriptClass)
{
	CHECK_LUA_STACK(m_luaState, 0);

	RegisteredClass rc;

	rc.scriptClass = scriptClass;

	lua_newtable(m_luaState);										// +1	-> 1
	rc.metaTableRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);		// -1	-> 0
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, rc.metaTableRef);	// +1	-> 1

	lua_pushvalue(m_luaState, -1);									// +1	-> 2
	lua_setfield(m_luaState, -2, "__index");						// -1	-> 1

	lua_pushlightuserdata(m_luaState, (void*)this);
	lua_pushlightuserdata(m_luaState, (void*)scriptClass);
	lua_pushcclosure(m_luaState, classEqualMethod, 2);
	lua_setfield(m_luaState, -2, "__eq");

	lua_pushlightuserdata(m_luaState, (void*)scriptClass);			// +1	-> 2
	lua_rawseti(m_luaState, -2, c_tableKey_class);					// -1	-> 1

	uint32_t methodCount = scriptClass->getMethodCount();
	for (uint32_t i = 0; i < methodCount; ++i)
	{
		std::wstring methodName = scriptClass->getMethodName(i);
		lua_pushinteger(m_luaState, i);
		lua_pushlightuserdata(m_luaState, (void*)this);
		lua_pushlightuserdata(m_luaState, (void*)scriptClass);
		lua_pushcclosure(m_luaState, classCallMethod, 3);
		lua_setfield(m_luaState, -2, wstombs(methodName).c_str());
	}

	const TypeInfo& exportType = scriptClass->getExportType();
	const TypeInfo* superType = exportType.getSuper();
	T_ASSERT (superType);

	lua_newtable(m_luaState);

	bool exportedAsRoot = true;
	for (std::vector< RegisteredClass >::iterator i = m_classRegistry.begin(); i != m_classRegistry.end(); ++i)
	{
		if (superType == &i->scriptClass->getExportType())
		{
			lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, i->metaTableRef);	// +1	-> 2
			lua_setmetatable(m_luaState, -2);								// -1	-> 1
			exportedAsRoot = false;
			break;
		}
	}

	if (exportedAsRoot && scriptClass->haveUnknown())
	{
		lua_pushlightuserdata(m_luaState, (void*)this);
		lua_pushlightuserdata(m_luaState, (void*)scriptClass);
		lua_pushcclosure(m_luaState, classIndexLookup, 2);
		lua_setfield(m_luaState, -2, "__index");
	}
	else if (!exportedAsRoot)
	{
		lua_pushvalue(m_luaState, -1);									// +1	-> 2
		lua_setfield(m_luaState, -2, "__index");						// -1	-> 1
	}

	if (scriptClass->haveConstructor())
	{
		lua_pushlightuserdata(m_luaState, (void*)this);				// +1	-> 2
		lua_pushlightuserdata(m_luaState, (void*)scriptClass);		// +1	-> 3
		lua_pushcclosure(m_luaState, classCallConstructor, 2);		// -1	-> 2
		lua_setfield(m_luaState, -2, "__call");						// -1	-> 1
	}

	lua_setmetatable(m_luaState, -2);

	// Export class in global scope.
	std::wstring exportName = exportType.getName();
	std::vector< std::wstring > exportPath;
	Split< std::wstring >::any(exportName, L".", exportPath);
	lua_setglobal(m_luaState, wstombs(exportPath.back()).c_str());

	m_classRegistryLookup[&exportType] = m_classRegistry.size();
	m_classRegistry.push_back(rc);
}

Ref< IScriptResource > ScriptManagerLua::compile(const std::wstring& script, bool strip, IErrorCallback* errorCallback) const
{
	return new ScriptResourceLua(wstombs(script));
}

Ref< IScriptContext > ScriptManagerLua::createContext()
{
	return new ScriptContextLua(this, m_luaState);
}

Ref< IScriptDebugger > ScriptManagerLua::createDebugger()
{
	return new ScriptDebuggerLua(this, m_luaState);
}

void ScriptManagerLua::lock(ScriptContextLua* context)
{
	m_lock.wait();
	m_currentContext = context;
}

void ScriptManagerLua::unlock()
{
	m_currentContext = 0;
	m_lock.release();
}

void ScriptManagerLua::pushObject(Object* object)
{
	CHECK_LUA_STACK(m_luaState, 1);

	if (!object)
	{
		lua_pushnil(m_luaState);
		return;
	}

	const TypeInfo* objectType = &type_of(object);

	// Find registered script class entry.
	SmallMap< const TypeInfo*, uint32_t >::const_iterator i = m_classRegistryLookup.find(objectType);
	while (i == m_classRegistryLookup.end())
	{
		if ((objectType = objectType->getSuper()) == 0)
			break;
		i = m_classRegistryLookup.find(objectType);
	}
	if (i == m_classRegistryLookup.end())
	{
		lua_pushnil(m_luaState);
		return;
	}

	const RegisteredClass& rc = m_classRegistry[i->second];

	//// Bind class meta table to instance.
	//lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, rc.metaTableRef);	// +1
	//lua_setfield(m_luaState, -2, "__class");						// -1

	// Create user data entry with C++ instance and GC callback.
	lua_newtable(m_luaState);						// +1

	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, rc.metaTableRef);	// +1
	lua_setmetatable(m_luaState, -2);								// -1

	Object** objectRef = reinterpret_cast< Object** >(lua_newuserdata(m_luaState, sizeof(Object*)));	// +1
	*objectRef = object;
	T_SAFE_ADDREF(*objectRef);

	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_gcMetaRef);
	lua_setmetatable(m_luaState, -2);

	lua_rawseti(m_luaState, -2, c_tableKey_this);	// -1
}

void ScriptManagerLua::pushAny(const Any& any)
{
	CHECK_LUA_STACK(m_luaState, 1);

	if (any.isBoolean())
		lua_pushboolean(m_luaState, any.getBoolean() ? 1 : 0);
	else if (any.isInteger())
		lua_pushinteger(m_luaState, any.getInteger());
	else if (any.isFloat())
		lua_pushnumber(m_luaState, any.getFloat());
	else if (any.isString())
		lua_pushstring(m_luaState, wstombs(any.getString()).c_str());
	else if (any.isObject())
		pushObject(any.getObject());
	else
		lua_pushnil(m_luaState);
}

Any ScriptManagerLua::toAny(int32_t index)
{
	CHECK_LUA_STACK(m_luaState, 0);

	if (lua_isnumber(m_luaState, index))
		return Any(float(lua_tonumber(m_luaState, index)));
	if (lua_isboolean(m_luaState, index))
		return Any(bool(lua_toboolean(m_luaState, index) != 0));
	if (lua_isstring(m_luaState, index))
		return Any(mbstows(lua_tostring(m_luaState, index)));
	if (lua_istable(m_luaState, index))
	{
		// Instance table; as C++ object.
		lua_rawgeti(m_luaState, index, c_tableKey_this);
		if (lua_isuserdata(m_luaState, -1))
		{
			Object* object = *reinterpret_cast< Object** >(lua_touserdata(m_luaState, -1));
			lua_pop(m_luaState, 1);
			if (object)
				return Any(object);
		}
		lua_pop(m_luaState, 1);

		// Class table; as C++ type.
		lua_rawgeti(m_luaState, index, c_tableKey_class);
		if (lua_isuserdata(m_luaState, -1))
		{
			const IScriptClass* scriptClass = reinterpret_cast< const IScriptClass* >(lua_touserdata(m_luaState, -1));
			lua_pop(m_luaState, 1);
			if (scriptClass)
				return Any(&scriptClass->getExportType());
		}
		lua_pop(m_luaState, 1);
	}

	return Any();
}

int ScriptManagerLua::classIndexLookup(lua_State* luaState)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT (manager);

	// Get script class from closure.
	const IScriptClass* scriptClass = reinterpret_cast< IScriptClass* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	if (!scriptClass)
		return 0;

	// Get index key.
	const char* key = lua_tostring(luaState, 2);
	T_ASSERT (key);

	// Create unknown method closure.
	lua_pushstring(luaState, key);
	lua_pushlightuserdata(luaState, (void*)manager);
	lua_pushlightuserdata(luaState, (void*)scriptClass);
	lua_pushcclosure(luaState, classCallUnknownMethod, 3);

	return 1;
}

int ScriptManagerLua::classCallConstructor(lua_State* luaState)
{
	Any argv[16];

	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT (manager);

	if (!lua_istable(luaState, 1))
		return 0;

	// Get script class from function.
	const IScriptClass* scriptClass = reinterpret_cast< IScriptClass* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	if (!scriptClass)
		return 0;

	// Convert arguments.
	int32_t argc = lua_gettop(luaState) - 1;
	T_ASSERT (argc <= sizeof_array(argv));
	for (int32_t i = 0; i < argc; ++i)
		argv[i] = manager->toAny(2 + i);

	// Invoke native method.
	IScriptClass::InvokeParam param;
	param.context = manager->m_currentContext;
	param.object = 0;

	Any returnValue(scriptClass->construct(param, argc, argv));
	manager->pushAny(returnValue);

	return 1;
}

int ScriptManagerLua::classCallMethod(lua_State* luaState)
{
	Any argv[16];

	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (manager);

	if (!lua_istable(luaState, 1))
		return 0;

	// Get script class from function.
	const IScriptClass* scriptClass = reinterpret_cast< IScriptClass* >(lua_touserdata(luaState, lua_upvalueindex(3)));
	if (!scriptClass)
		return 0;

	// Get object pointer.
	lua_rawgeti(luaState, 1, c_tableKey_this);
	Object* object = *reinterpret_cast< Object** >(lua_touserdata(luaState, -1));
	lua_pop(luaState, 1);
	if (!object)
		return 0;

	// Get called method id from closure.
	int32_t methodId = (int32_t)lua_tonumber(luaState, lua_upvalueindex(1));

	// Convert arguments.
	int32_t argc = lua_gettop(luaState) - 1;
	T_ASSERT (argc <= sizeof_array(argv));
	for (int32_t i = 0; i < argc; ++i)
		argv[i] = manager->toAny(2 + i);

	// Invoke native method.
	IScriptClass::InvokeParam param;
	param.context = manager->m_currentContext;
	param.object = object;

	Any returnValue = scriptClass->invoke(param, methodId, argc, argv);
	manager->pushAny(returnValue);

	return 1;
}

int ScriptManagerLua::classCallUnknownMethod(lua_State* luaState)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (manager);

	Any argv[16];

	if (!lua_istable(luaState, 1))
		return 0;

	const char* methodName = lua_tostring(luaState, lua_upvalueindex(1));
	T_ASSERT (methodName);

	const IScriptClass* scriptClass = reinterpret_cast< IScriptClass* >(lua_touserdata(luaState, lua_upvalueindex(3)));
	if (!scriptClass)
		return 0;

	// Get object pointer.
	lua_rawgeti(luaState, 1, c_tableKey_this);
	Object* object = *reinterpret_cast< Object** >(lua_touserdata(luaState, -1));
	lua_pop(luaState, 1);
	if (!object)
		return 0;

	// Convert arguments.
	int32_t argc = lua_gettop(luaState) - 1;
	T_ASSERT (argc <= sizeof_array(argv));
	for (int32_t i = 0; i < argc; ++i)
		argv[i] = manager->toAny(2 + i);

	// Invoke native method.
	IScriptClass::InvokeParam param;
	param.context = manager->m_currentContext;
	param.object = object;

	Any returnValue = scriptClass->invokeUnknown(param, mbstows(methodName), argc, argv);
	manager->pushAny(returnValue);

	return 1;
}

int ScriptManagerLua::classGcMethod(lua_State* luaState)
{
	Object* object = *reinterpret_cast< Object** >(lua_touserdata(luaState, 1));
	T_SAFE_ANONYMOUS_RELEASE(object);
	return 0;
}

int ScriptManagerLua::classEqualMethod(lua_State* luaState)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT (manager);

	Any object0 = manager->toAny(1);
	Any object1 = manager->toAny(2);

	if (object0.isObject() && object1.isObject())
	{
		if (object0.getObject() == object1.getObject())
			return 1;
	}

	return 0;
}

int ScriptManagerLua::luaPanic(lua_State* luaState)
{
	log::error << L"LUA PANIC; Unrecoverable error \"" << mbstows(lua_tostring(luaState, lua_gettop(luaState))) << L"\"" << Endl;
	return 0;
}

	}
}
