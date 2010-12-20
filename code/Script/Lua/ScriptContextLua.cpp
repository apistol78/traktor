#include <csetjmp>
#include "Core/Log/Log.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Misc/TString.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Acquire.h"
#include "Script/IScriptClass.h"
#include "Script/Lua/ScriptContextLua.h"
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
		namespace
		{

const int32_t c_tableKey_class = -1;
const int32_t c_tableKey_this = -2;

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

#if defined(_DEBUG)
void stackDump(lua_State* luaState)
{
	int32_t top = lua_gettop(luaState);

	log::debug << L"Total in stack " << top << Endl;

	for (int32_t i = 1; i <= top; ++i)
	{
		int t = lua_type(luaState, i);
		switch (t)
		{
		case LUA_TSTRING:
			log::debug << L"\tstring: \"" << mbstows(lua_tostring(luaState, i)) << L"\"" << Endl;
			break;

		case LUA_TBOOLEAN:
			log::debug << L"\tboolean: " << (lua_toboolean(luaState, i) ? L"true" : L"false") << Endl;
			break;

		case LUA_TNUMBER:
			log::debug << L"\tnumber: " << lua_tonumber(luaState, i) << Endl;
			break;

		default:  /* other values */
			log::debug << L"\tother: " << lua_typename(luaState, t) << Endl;
			break;
		}
	}
}
#	define DUMP_LUA_STACK(state) stackDump(state)
#else
#	define DUMP_LUA_STACK(state)
#endif

void* luaAlloc(void* ud, void* ptr, size_t osize, size_t nsize)
{
	if (osize >= nsize)
		return ptr;

	if (nsize > 0)
	{
		void* nptr = getAllocator()->alloc(nsize, 16, "LUA");
		if (!nptr)
			return 0;

		if (ptr && osize > 0)
		{
			std::memcpy(nptr, ptr, std::min(osize, nsize));
			getAllocator()->free(ptr);
		}

		return nptr;
	}
	else
	{
		getAllocator()->free(ptr);
		return 0;
	}
}

int luaPrint(lua_State *L) 
{
	int n = lua_gettop(L);
	int i;

	lua_getglobal(L, "tostring");
	for (i = 1; i <= n; ++i)
	{
		const char* s;
		lua_pushvalue(L, -1);
		lua_pushvalue(L, i);
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1);
		if (s == NULL)
			return luaL_error(L, LUA_QL("tostring") " must return a string to " LUA_QL("print"));
		if (i > 1)
			log::info << L"\t";
		log::info << mbstows(s);
		lua_pop(L, 1);
	}

	log::info << Endl;
	return 0;
}

std::jmp_buf s_jb;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptContextLua", ScriptContextLua, IScriptContext)

ScriptContextLua::ScriptContextLua(const RefArray< IScriptClass >& registeredClasses)
:	m_luaState(0)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	m_luaState = lua_newstate(&luaAlloc, 0);

	lua_atpanic(m_luaState, luaPanic);
	lua_gc(m_luaState, LUA_GCSTOP, 0);

	luaopen_base(m_luaState);
	luaopen_table(m_luaState);
	luaopen_string(m_luaState);
	luaopen_math(m_luaState);

	lua_register(m_luaState, "print", luaPrint);

	for (RefArray< IScriptClass >::const_iterator i = registeredClasses.begin(); i != registeredClasses.end(); ++i)
		registerClass(*i);
		
	lua_gc(m_luaState, LUA_GCRESTART, 0);
}

ScriptContextLua::~ScriptContextLua()
{
	destroy();
}

void ScriptContextLua::destroy()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	if (m_luaState)
	{
		lua_close(m_luaState);
		m_luaState = 0;
	}
}

void ScriptContextLua::setGlobal(const std::wstring& globalName, const Any& globalValue)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	CHECK_LUA_STACK(m_luaState, 0);
	pushAny(globalValue);
	lua_setglobal(m_luaState, wstombs(globalName).c_str());
}

Any ScriptContextLua::getGlobal(const std::wstring& globalName)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	CHECK_LUA_STACK(m_luaState, 0);
	lua_getglobal(m_luaState, wstombs(globalName).c_str());
	return toAny(-1);
}

bool ScriptContextLua::executeScript(const IScriptResource* scriptResource)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	CHECK_LUA_STACK(m_luaState, 0);

	const ScriptResourceLua* scriptResourceLua = checked_type_cast< const ScriptResourceLua*, false >(scriptResource);
	int32_t result = luaL_loadstring(m_luaState, wstombs(scriptResourceLua->getScript()).c_str());
	if (result != 0)
		return false;

	lua_pcall(m_luaState, 0, LUA_MULTRET, 0);
	return true;
}

bool ScriptContextLua::haveFunction(const std::wstring& functionName) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
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

Any ScriptContextLua::executeFunction(const std::wstring& functionName, uint32_t argc, const Any* argv)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	CHECK_LUA_STACK(m_luaState, 0);

	Any returnValue;

#if defined(_PS3)
	if (std::setjmp(s_jb) == 0)
#else
	if (setjmp(s_jb) == 0)
#endif
	{
		lua_getglobal(m_luaState, wstombs(functionName).c_str());
		if (!lua_isnil(m_luaState, -1))
		{
			// Push arguments; prevent GC while pushing arguments.
			lua_gc(m_luaState, LUA_GCSTOP, 0);
			for (uint32_t i = 0; i < argc; ++i)
				pushAny(argv[i]);
			lua_gc(m_luaState, LUA_GCRESTART, 0);
			
			lua_call(m_luaState, argc, 1);
			returnValue = toAny(-1);
		}
		lua_pop(m_luaState, 1);
	}

	return returnValue;
}

Any ScriptContextLua::executeMethod(Object* self, const std::wstring& methodName, uint32_t argc, const Any* argv)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	CHECK_LUA_STACK(m_luaState, 0);

	Any returnValue;

#if defined(_PS3)
	if (std::setjmp(s_jb) == 0)
#else
	if (setjmp(s_jb) == 0)
#endif
	{
		lua_getglobal(m_luaState, wstombs(methodName).c_str());
		if (!lua_isnil(m_luaState, -1))
		{
			// Push arguments; prevent GC while pushing arguments.
			lua_gc(m_luaState, LUA_GCSTOP, 0);
			pushAny(Any(self));
			for (uint32_t i = 0; i < argc; ++i)
				pushAny(argv[i]);
			lua_gc(m_luaState, LUA_GCRESTART, 0);

			lua_call(m_luaState, int32_t(argc + 1), 1);
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

	lua_newtable(m_luaState);										// +1	-> 1
	rc.metaTableRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);		// -1	-> 0
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, rc.metaTableRef);	// +1	-> 1

	lua_pushvalue(m_luaState, -1);									// +1	-> 2
	lua_setfield(m_luaState, -2, "__index");						// -1	-> 1

	lua_pushlightuserdata(m_luaState, (void*)scriptClass);			// +1	-> 2
	lua_rawseti(m_luaState, -2, c_tableKey_class);					// -1	-> 1

	if (scriptClass->haveConstructor())
	{
		lua_pushlightuserdata(m_luaState, (void*)this);				// +1	-> 2
		lua_pushlightuserdata(m_luaState, (void*)scriptClass);		// +1	-> 3
		lua_pushcclosure(m_luaState, callConstructor, 2);			// -1	-> 2
		lua_setfield(m_luaState, -2, "new");						// -1	-> 1
	}

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

	const TypeInfo& exportType = scriptClass->getExportType();
	const TypeInfo* superType = exportType.getSuper();
	T_ASSERT (superType);

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

	if (exportedAsRoot)
	{
		lua_newtable(m_luaState);

		lua_pushlightuserdata(m_luaState, (void*)this);
		lua_pushlightuserdata(m_luaState, (void*)scriptClass);
		lua_pushcclosure(m_luaState, classIndexLookup, 2);
		lua_setfield(m_luaState, -2, "__index");

		lua_setmetatable(m_luaState, -2);
	}

	if (scriptClass->haveConstructor())
	{
		std::wstring exportName = exportType.getName();

		std::vector< std::wstring > exportPath;
		Split< std::wstring >::any(exportName, L".", exportPath);

		lua_setglobal(m_luaState, wstombs(exportPath.back()).c_str());
	}
	else
		lua_pop(m_luaState, 1);

	m_classRegistryLookup[&exportType] = m_classRegistry.size();
	m_classRegistry.push_back(rc);
}

void ScriptContextLua::pushObject(Object* object)
{
	CHECK_LUA_STACK(m_luaState, 1);

	if (!object)
	{
		lua_pushnil(m_luaState);
		return;
	}

	const TypeInfo* objectType = &type_of(object);

	// Find registered script class entry.
	std::map< const TypeInfo*, uint32_t >::const_iterator i = m_classRegistryLookup.find(objectType);
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

	lua_newtable(m_luaState);						// +1

	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, rc.metaTableRef);	// +1
	lua_setmetatable(m_luaState, -2);								// -1

	// Associate object with instance table.
	Object** objectRef = reinterpret_cast< Object** >(lua_newuserdata(m_luaState, sizeof(Object*)));	// +1
	*objectRef = object;
	T_SAFE_ADDREF(*objectRef);

	// Associate __gc with object userdata.
	lua_newtable(m_luaState);						// +1
	lua_pushcfunction(m_luaState, gcMethod);		// +1

	lua_setfield(m_luaState, -2, "__gc");			// -1
	lua_setmetatable(m_luaState, -2);				// -1

	lua_rawseti(m_luaState, -2, c_tableKey_this);	// -1

	//// Expose properties.
	//uint32_t propertyCount = i->scriptClass->getPropertyCount();
	//for (uint32_t j = 0; j < propertyCount; ++j)
	//{
	//	std::wstring propertyName = i->scriptClass->getPropertyName(j);
	//	lua_pushlightuserdata(m_luaState, (void*)this);
	//	lua_pushlightuserdata(m_luaState, (void*)i->scriptClass);
	//	lua_pushcclosure(m_luaState, callProperty, 2);
	//	lua_setfield(m_luaState, -2, wstombs(propertyName).c_str());
	//}
}

void ScriptContextLua::pushAny(const Any& any)
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

Any ScriptContextLua::toAny(int32_t index)
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
		lua_rawgeti(m_luaState, index, c_tableKey_this);

		Object* object = *reinterpret_cast< Object** >(lua_touserdata(m_luaState, -1));
		lua_pop(m_luaState, 1);
		if (object)
			return Any(object);
	}
	
	return Any();
}

int ScriptContextLua::classIndexLookup(lua_State* luaState)
{
	ScriptContextLua* context = reinterpret_cast< ScriptContextLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT (context);

	// Get script class from closure.
	const IScriptClass* scriptClass = reinterpret_cast< IScriptClass* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	if (!scriptClass)
		return 0;

	// Get index key.
	const char* key = lua_tostring(luaState, 2);
	T_ASSERT (key);

	// Create unknown method closure.
	lua_pushstring(luaState, key);
	lua_pushlightuserdata(luaState, (void*)context);
	lua_pushlightuserdata(luaState, (void*)scriptClass);
	lua_pushcclosure(luaState, callUnknownMethod, 3);

	return 1;
}

int ScriptContextLua::callConstructor(lua_State* luaState)
{
	Any argv[16];

	ScriptContextLua* context = reinterpret_cast< ScriptContextLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT (context);

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
		argv[i] = context->toAny(2 + i);

	// Invoke native method.
	IScriptClass::InvokeParam param;
	param.context = context;
	param.object = 0;

	Any returnValue(scriptClass->construct(param, argc, argv));
	context->pushAny(returnValue);

	return 1;
}

int ScriptContextLua::callMethod(lua_State* luaState)
{
	Any argv[16];

	ScriptContextLua* context = reinterpret_cast< ScriptContextLua* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (context);

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
		argv[i] = context->toAny(2 + i);

	// Invoke native method.
	IScriptClass::InvokeParam param;
	param.context = context;
	param.object = object;

	Any returnValue = scriptClass->invoke(param, methodId, argc, argv);
	context->pushAny(returnValue);

	return 1;
}

int ScriptContextLua::callUnknownMethod(lua_State* luaState)
{
	ScriptContextLua* context = reinterpret_cast< ScriptContextLua* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (context);

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
		argv[i] = context->toAny(2 + i);

	// Invoke native method.
	IScriptClass::InvokeParam param;
	param.context = context;
	param.object = object;

	Any returnValue = scriptClass->invokeUnknown(param, mbstows(methodName), argc, argv);
	context->pushAny(returnValue);

	return 1;
}

int ScriptContextLua::callProperty(lua_State* luaState)
{
	ScriptContextLua* context = reinterpret_cast< ScriptContextLua* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (context);

	if (!lua_istable(luaState, 1))
		return 0;

	int32_t propertyId = lua_tointeger(luaState, lua_upvalueindex(1));

	const IScriptClass* scriptClass = reinterpret_cast< IScriptClass* >(lua_touserdata(luaState, lua_upvalueindex(3)));
	if (!scriptClass)
		return 0;

	// Get object pointer.
	lua_rawgeti(luaState, 1, c_tableKey_this);
	Object* object = *reinterpret_cast< Object** >(lua_touserdata(luaState, -1));
	lua_pop(luaState, 1);
	if (!object)
		return 0;

	int32_t argc = lua_gettop(luaState) - 1;
	if (argc == 2)
	{
		IScriptClass::InvokeParam param;
		param.context = context;
		param.object = object;

		// Get property value.
		Any propertyValue = scriptClass->getPropertyValue(param, propertyId);
		context->pushAny(propertyValue);
		return 1;
	}
	else if (argc == 3)
	{
		// Set property value.
		return 0;
	}

	return 0;
}

int ScriptContextLua::gcMethod(lua_State* luaState)
{
	Object* object = *reinterpret_cast< Object** >(lua_touserdata(luaState, 1));
	T_SAFE_ANONYMOUS_RELEASE(object);
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
