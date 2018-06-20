/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClass.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Save.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Thread/Acquire.h"
#include "Core/Timer/Timer.h"
#include "Script/Lua/ScriptBlobLua.h"
#include "Script/Lua/ScriptClassLua.h"
#include "Script/Lua/ScriptContextLua.h"
#include "Script/Lua/ScriptDebuggerLua.h"
#include "Script/Lua/ScriptDelegateLua.h"
#include "Script/Lua/ScriptManagerLua.h"
#include "Script/Lua/ScriptObjectLua.h"
#include "Script/Lua/ScriptProfilerLua.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

// Resources
#include "Resources/Initialization.h"

#if defined(T_LUA_5_2) || (!defined(__LP64__) && !defined(_LP64))
#	define T_USE_ALLOCATOR 1
#endif

namespace traktor
{
	namespace script
	{
		namespace
		{

Timer s_timer;

const int32_t c_tableKey_class = -1;
const int32_t c_tableKey_instance = -2;

ITypedObject* toTypedObject(lua_State* luaState, int32_t index)
{
	lua_rawgeti(luaState, index, c_tableKey_instance);
	if (!lua_islightuserdata(luaState, -1))
	{
		lua_pop(luaState, 1);
		return 0;
	}

	ITypedObject* object = reinterpret_cast< ITypedObject* >(lua_touserdata(luaState, -1));
	if (!object)
	{
		lua_pop(luaState, 1);
		return 0;
	}

	lua_pop(luaState, 1);
	return object;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptManagerLua", 0, ScriptManagerLua, IScriptManager)

ScriptManagerLua::ScriptManagerLua()
:	m_luaState(0)
,	m_defaultAllocFn(0)
,	m_defaultAllocOpaque(0)
,	m_lockContext(0)
,	m_collectStepFrequency(10.0)
,	m_collectSteps(-1)
,	m_collectTargetSteps(0.0f)
,	m_totalMemoryUse(0)
,	m_lastMemoryUse(0)
{
#if defined(T_USE_ALLOCATOR)
	m_luaState = lua_newstate(&luaAlloc, this);
#else
	m_luaState = luaL_newstate();

	// Hook default allocator to intercept allocation stats.
	m_defaultAllocFn = (void*)lua_getallocf(m_luaState, &m_defaultAllocOpaque);
	T_FATAL_ASSERT (m_defaultAllocFn);
	lua_setallocf(m_luaState, &luaAlloc, this);
#endif

	lua_atpanic(m_luaState, luaPanic);

#if defined(T_LUA_5_2)
	luaL_openlibs(m_luaState);
#else
	luaopen_base(m_luaState);
	luaopen_table(m_luaState);
	luaopen_string(m_luaState);
	luaopen_math(m_luaState);
	luaopen_os(m_luaState);
#	if defined(LUA_BITLIBNAME)
	luaopen_bit(m_luaState);
#	endif
#endif

	lua_register(m_luaState, "print", luaPrint);
	lua_register(m_luaState, "sleep", luaSleep);

	lua_pushlightuserdata(m_luaState, (void*)this);
	lua_pushcclosure(m_luaState, luaAllocatedMemory, 1);
	lua_setglobal(m_luaState, "allocatedMemory");

	// Load default initialization script(s).
	luaL_loadbuffer(
		m_luaState,
		reinterpret_cast< const char* >(c_ResourceInitialization),
		sizeof(c_ResourceInitialization),
		"init"
	);
	lua_pcall(m_luaState, 0, 0, 0);

	// Create table containing weak references to C++ object wrappers.
	{
		CHECK_LUA_STACK(m_luaState, 0);

		lua_newtable(m_luaState);
		m_objectTableRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_objectTableRef);

		lua_newtable(m_luaState);
		lua_pushstring(m_luaState, "kv");
		lua_setfield(m_luaState, -2, "__mode");
		lua_setmetatable(m_luaState, -2);

		lua_pop(m_luaState, 1);
	}

	s_timer.start();
}

ScriptManagerLua::~ScriptManagerLua()
{
	T_FATAL_ASSERT_M(!m_luaState, L"Must call destroy");
}

void ScriptManagerLua::destroy()
{
	if (!m_luaState)
		return;

	T_ANONYMOUS_VAR(Ref< ScriptManagerLua >)(this);

	// Discard all tags from C++ rtti types.
	for (AlignedVector< RegisteredClass >::iterator i = m_classRegistry.begin(); i != m_classRegistry.end(); ++i)
	{
		TypeInfoSet derivedTypes;
		i->runtimeClass->getExportType().findAllOf(derivedTypes);
		for (TypeInfoSet::iterator j = derivedTypes.begin(); j != derivedTypes.end(); ++j)
			(*j)->setTag(0);
	}

	m_debugger = 0;
	m_profiler = 0;

	while (!m_contexts.empty())
		m_contexts.back()->destroy();

	luaL_unref(m_luaState, LUA_REGISTRYINDEX, m_objectTableRef);
	m_objectTableRef = LUA_NOREF;

	lua_close(m_luaState);
	m_luaState = 0;
}

void ScriptManagerLua::registerClass(IRuntimeClass* runtimeClass)
{
	T_ANONYMOUS_VAR(UnwindStack)(m_luaState);

	const TypeInfo& exportType = runtimeClass->getExportType();
	int32_t classRegistryIndex = int32_t(m_classRegistry.size());
	
	RegisteredClass& rc = m_classRegistry.push_back();
	rc.runtimeClass = runtimeClass;

	// Create new class.
	lua_getglobal(m_luaState, "class");
	T_FATAL_ASSERT (lua_isfunction(m_luaState, -1));
	lua_pushstring(m_luaState, wstombs(exportType.getName()).c_str());
	if (exportType.getSuper())
	{
		int32_t superClassId = int32_t(exportType.getSuper()->getTag()) - 1;
		if (superClassId >= 0)
		{
			const RegisteredClass& superClass = m_classRegistry[superClassId];
			lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, superClass.classTableRef);
		}
		else
			lua_pushnil(m_luaState);
	}
	else
		lua_pushnil(m_luaState);
	lua_call(m_luaState, 2, 1);
	T_FATAL_ASSERT (lua_istable(m_luaState, -1));

	// Attach C++ runtime class to script table.
	lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
	lua_rawseti(m_luaState, -2, c_tableKey_class);

	// Create "__alloc" callback to be able to instantiate C++ object when creating from script side.
	lua_pushlightuserdata(m_luaState, (void*)this);
	lua_pushinteger(m_luaState, classRegistryIndex);
	lua_pushcclosure(m_luaState, classAlloc, 2);
	lua_setfield(m_luaState, -2, "__alloc");

	// Create "__gc" callback to be able to track C++ object lifetime.
	lua_pushcfunction(m_luaState, classGc);
	lua_setfield(m_luaState, -2, "__gc");

	// Create "new" callback to be able to instantiate C++ object when creating from script side.
	lua_pushlightuserdata(m_luaState, (void*)this);
	lua_pushinteger(m_luaState, classRegistryIndex);
	lua_pushcclosure(m_luaState, classNew, 2);
	lua_setfield(m_luaState, -2, "new");

	// Create "__unknown" callback if class handle unknown calls.
	if (runtimeClass->haveUnknown())
	{
		lua_pushlightuserdata(m_luaState, (void*)this);
		lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
		lua_pushcclosure(m_luaState, classCallUnknownMethod, 2);
		lua_setfield(m_luaState, -2, "__unknown");
	}

	// Add static methods.
	uint32_t staticMethodCount = runtimeClass->getStaticMethodCount();
	for (uint32_t i = 0; i < staticMethodCount; ++i)
	{
		std::string methodName = runtimeClass->getStaticMethodName(i);
		lua_pushinteger(m_luaState, i);
		lua_pushlightuserdata(m_luaState, (void*)this);
		lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
		lua_pushcclosure(m_luaState, classCallStaticMethod, 3);
		lua_setfield(m_luaState, -2, methodName.c_str());
	}

	// Add methods.
	uint32_t methodCount = runtimeClass->getMethodCount();
	for (uint32_t i = 0; i < methodCount; ++i)
	{
		std::string methodName = runtimeClass->getMethodName(i);
		lua_pushinteger(m_luaState, i);
		lua_pushlightuserdata(m_luaState, (void*)this);
		lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
		lua_pushcclosure(m_luaState, classCallMethod, 3);
		lua_setfield(m_luaState, -2, methodName.c_str());
	}

	// Add properties.
	T_FATAL_ASSERT (lua_istable(m_luaState, - 1));
	uint32_t propertyCount = runtimeClass->getPropertiesCount();
	for (uint32_t i = 0; i < propertyCount; ++i)
	{
		std::string propertyName = runtimeClass->getPropertyName(i);

		lua_getfield(m_luaState, -1, "__setters");
		T_FATAL_ASSERT (lua_istable(m_luaState, - 1));

		lua_pushinteger(m_luaState, i);
		lua_pushlightuserdata(m_luaState, (void*)this);
		lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
		lua_pushcclosure(m_luaState, classSetProperty, 3);
		T_FATAL_ASSERT (lua_isfunction(m_luaState, - 1));

		lua_setfield(m_luaState, -2, propertyName.c_str());
		lua_pop(m_luaState, 1);

		lua_getfield(m_luaState, -1, "__getters");
		T_FATAL_ASSERT (lua_istable(m_luaState, - 1));
		
		lua_pushinteger(m_luaState, i);
		lua_pushlightuserdata(m_luaState, (void*)this);
		lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
		lua_pushcclosure(m_luaState, classGetProperty, 3);
		T_FATAL_ASSERT (lua_isfunction(m_luaState, - 1));

		lua_setfield(m_luaState, -2, propertyName.c_str());
		lua_pop(m_luaState, 1);
	}

	// Add operators.
	lua_pushlightuserdata(m_luaState, (void*)this);
	lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
	lua_pushcclosure(m_luaState, classEqual, 2);
	lua_setfield(m_luaState, -2, "__eq");

	lua_pushlightuserdata(m_luaState, (void*)this);
	lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
	lua_pushcclosure(m_luaState, classAdd, 2);
	lua_setfield(m_luaState, -2, "__add");

	lua_pushlightuserdata(m_luaState, (void*)this);
	lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
	lua_pushcclosure(m_luaState, classSubtract, 2);
	lua_setfield(m_luaState, -2, "__sub");

	lua_pushlightuserdata(m_luaState, (void*)this);
	lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
	lua_pushcclosure(m_luaState, classMultiply, 2);
	lua_setfield(m_luaState, -2, "__mul");

	lua_pushlightuserdata(m_luaState, (void*)this);
	lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
	lua_pushcclosure(m_luaState, classDivide, 2);
	lua_setfield(m_luaState, -2, "__div");

	rc.classTableRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);

	// Export class in global scope.
	std::wstring exportName = exportType.getName();
	std::vector< std::wstring > exportPath;
	Split< std::wstring >::any(exportName, L".", exportPath);

#if defined(T_LUA_5_2)

	lua_pushglobaltable(m_luaState);

	if (exportPath.size() > 1)
	{
		for (size_t i = 0; i < exportPath.size() - 1; ++i)
		{
			lua_getfield(m_luaState, -1, wstombs(exportPath[i]).c_str());
			if (!lua_istable(m_luaState, -1))
			{
				lua_pop(m_luaState, 1);
				lua_newtable(m_luaState);
				lua_setfield(m_luaState, -2, wstombs(exportPath[i]).c_str());
				lua_getfield(m_luaState, -1, wstombs(exportPath[i]).c_str());
				T_ASSERT (lua_istable(m_luaState, -1));
			}
			else
				lua_replace(m_luaState, -2);
		}
	}

	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, rc.classTableRef);
	lua_setfield(m_luaState, -2, wstombs(exportPath.back()).c_str());

	lua_pop(m_luaState, 1);

#else
	// \fixme!

	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, rc.classTableRef);
	lua_setglobal(m_luaState, wstombs(exportPath.back()).c_str());

#endif

	// Store index of registered script class in C++ rtti type; used
	// to accelerate lookup of C++ class when constructing new instance from script.
	// Need to propagate index into derived types as well in order to
	// be able to skip traversing class hierarchy while constructing.
	TypeInfoSet derivedTypes;
	exportType.findAllOf(derivedTypes);
	for (TypeInfoSet::iterator i = derivedTypes.begin(); i != derivedTypes.end(); ++i)
	{
		if ((*i)->getTag() != 0)
		{
			const RegisteredClass& rc2 = m_classRegistry[(*i)->getTag() - 1];
			const TypeInfo& exportType2 = rc2.runtimeClass->getExportType();
			if (is_type_of(exportType, exportType2))
				continue;
		}
		(*i)->setTag(classRegistryIndex + 1);
	}

	// Add constants last as constants might be instances of this class, i.e. singletons etc.
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, rc.classTableRef);
	for (uint32_t i = 0; i < runtimeClass->getConstantCount(); ++i)
	{
		pushAny(runtimeClass->getConstantValue(i));
		lua_setfield(m_luaState, -2, runtimeClass->getConstantName(i).c_str());
	}
}

Ref< IScriptBlob > ScriptManagerLua::compile(const std::wstring& fileName, const std::wstring& script, IErrorCallback* errorCallback) const
{
#if defined(T_SCRIPT_LUA_USE_MT_LOCK)
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
#endif
	CHECK_LUA_STACK(m_luaState, 0);

	std::string metaFileName = "@" + wstombs(Utf8Encoding(), fileName);
	std::string text = wstombs(Utf8Encoding(), script);

	int32_t result = luaL_loadbuffer(
		m_luaState,
		text.c_str(),
		text.length(),
		metaFileName.c_str()
	);
	if (result != 0)
	{
		std::wstring error = mbstows(lua_tostring(m_luaState, -1));

		size_t p0 = error.find(L':');
		T_ASSERT (p0 != error.npos);

		error = error.substr(p0 + 1);

		size_t p1 = error.find(L':');
		T_ASSERT (p1 != error.npos);

		int32_t line = parseString< int32_t >(error.substr(0, p1));
		error = trim(error.substr(p1 + 1));

		if (errorCallback)
			errorCallback->syntaxError(fileName, line, error);
		else
			log::error << fileName << L" (" << line << L"): " << error << Endl;

		lua_pop(m_luaState, 1);
		return 0;
	}

	lua_pop(m_luaState, 1);

	Ref< ScriptBlobLua > blob = new ScriptBlobLua();
	blob->m_fileName = wstombs(fileName);
	blob->m_script = text;

	return blob;
}

Ref< IScriptContext > ScriptManagerLua::createContext(bool strict)
{
#if defined(T_SCRIPT_LUA_USE_MT_LOCK)
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
#endif
	CHECK_LUA_STACK(m_luaState, 0);

	// Create local environment table and add to registry.
	lua_newtable(m_luaState);
	int32_t environmentRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, environmentRef);

	// Create table with __index as global environment.
	lua_newtable(m_luaState);
	lua_getglobal(m_luaState, "_G");
	lua_setfield(m_luaState, -2, "__index");

	// Setup "inheritance" with the global environment.
	lua_setmetatable(m_luaState, -2);
	lua_pop(m_luaState, 1);

	// Create context.
	Ref< ScriptContextLua > context = new ScriptContextLua(this, m_luaState, environmentRef, strict);
	m_contexts.push_back(context);
	return context;
}

Ref< IScriptDebugger > ScriptManagerLua::createDebugger()
{
#if defined(T_SCRIPT_LUA_USE_MT_LOCK)
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
#endif

	if (!m_debugger)
		m_debugger = new ScriptDebuggerLua(this, m_luaState);

	lua_sethook(m_luaState, &ScriptManagerLua::hookCallback, LUA_MASKLINE, 0);
	return m_debugger;
}

Ref< IScriptProfiler > ScriptManagerLua::createProfiler()
{
#if defined(T_SCRIPT_LUA_USE_MT_LOCK)
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
#endif

	if (!m_profiler)
		m_profiler = new ScriptProfilerLua(this, m_luaState);

	lua_sethook(m_luaState, &ScriptManagerLua::hookCallback, LUA_MASKLINE | LUA_MASKCALL | LUA_MASKRET, 0);
	return m_profiler;
}

void ScriptManagerLua::collectGarbage(bool full)
{
	if (!full)
		collectGarbagePartial();
	else
		collectGarbageFull();
}

void ScriptManagerLua::getStatistics(ScriptStatistics& outStatistics) const
{
	outStatistics.memoryUsage = uint32_t(m_totalMemoryUse);
}

void ScriptManagerLua::destroyContext(ScriptContextLua* context)
{
	m_contexts.remove(context);
}

void ScriptManagerLua::pushObject(ITypedObject* object)
{
	CHECK_LUA_STACK(m_luaState, 1);

	if (!object)
	{
		lua_pushnil(m_luaState);
		return;
	}

	// If this is a wrapped LUA object or function then unwrap and push as is.
	const TypeInfo& objectType = type_of(object);
	if (&objectType == &type_of< ScriptObjectLua >())
	{
		ScriptObjectLua* scriptObject = checked_type_cast< ScriptObjectLua*, false >(object);
		scriptObject->push();
		return;
	}
	else if (&objectType == &type_of< ScriptDelegateLua >())
	{
		ScriptDelegateLua* delegateContainer = checked_type_cast< ScriptDelegateLua*, false >(object);
		delegateContainer->push();
		return;
	}

	// Have we already pushed this object before and it's still alive in script-land then reuse it.
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_objectTableRef);
	lua_rawgeti(m_luaState, -1, int32_t(uint64_t(object)));
	if (lua_istable(m_luaState, -1))
	{
		lua_remove(m_luaState, -2);
		return;
	}
	lua_pop(m_luaState, 2);

	// Get class index.
	uint32_t classId = 0;
	if (objectType.getTag() != 0)
		classId = objectType.getTag() - 1;
	else
	{
		lua_pushnil(m_luaState);
		return;
	}

	const RegisteredClass& rc = m_classRegistry[classId];

	// Create table to act as object instance in script-land.
	lua_newtable(m_luaState);

	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, rc.classTableRef);
	lua_setmetatable(m_luaState, -2);

	// Attach native object as light user value of table.
	lua_pushlightuserdata(m_luaState, (void*)object);
	lua_rawseti(m_luaState, -2, c_tableKey_instance);
	T_SAFE_ADDREF(object);

	// Store object instance in weak table.
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_objectTableRef);
	lua_insert(m_luaState, -2);
	lua_rawseti(m_luaState, -2, int32_t(uint64_t(object)));
	lua_rawgeti(m_luaState, -1, int32_t(uint64_t(object)));
	lua_remove(m_luaState, -2);
}

void ScriptManagerLua::pushAny(const Any& any)
{
	CHECK_LUA_STACK(m_luaState, 1);
	if (any.isBoolean())
		lua_pushboolean(m_luaState, any.getBooleanUnsafe() ? 1 : 0);
	else if (any.isInt32())
		lua_pushinteger(m_luaState, any.getInt32Unsafe());
	else if (any.isInt64())
		lua_pushinteger(m_luaState, any.getInt64Unsafe());
	else if (any.isFloat())
		lua_pushnumber(m_luaState, any.getFloatUnsafe());
	else if (any.isString())
		lua_pushstring(m_luaState, any.getStringUnsafe().c_str());
	else if (any.isObject())
		pushObject(any.getObjectUnsafe());
	else
		lua_pushnil(m_luaState);
}

void ScriptManagerLua::pushAny(const Any* anys, int32_t count)
{
	CHECK_LUA_STACK(m_luaState, count);
	for (int32_t i = 0; i < count; ++i)
	{
		const Any& any = anys[i];
		if (any.isBoolean())
			lua_pushboolean(m_luaState, any.getBooleanUnsafe() ? 1 : 0);
		else if (any.isInt32())
			lua_pushinteger(m_luaState, any.getInt32Unsafe());
		else if (any.isInt64())
			lua_pushinteger(m_luaState, any.getInt64Unsafe());
		else if (any.isFloat())
			lua_pushnumber(m_luaState, any.getFloatUnsafe());
		else if (any.isString())
			lua_pushstring(m_luaState, any.getStringUnsafe().c_str());
		else if (any.isObject())
			pushObject(any.getObjectUnsafe());
		else
			lua_pushnil(m_luaState);
	}
}

Any ScriptManagerLua::toAny(int32_t index)
{
	CHECK_LUA_STACK(m_luaState, 0);

	int32_t type = lua_type(m_luaState, index);
	if (type == LUA_TNUMBER)
	{
#if defined(T_LUA_5_2)
		if (lua_isinteger(m_luaState, index))
			return Any::fromInt64(lua_tointeger(m_luaState, index));
		else
#endif
			return Any::fromFloat(float(lua_tonumber(m_luaState, index)));
	}
	else if (type == LUA_TBOOLEAN)
		return Any::fromBoolean(bool(lua_toboolean(m_luaState, index) != 0));
	else if (type == LUA_TSTRING)
		return Any::fromString(lua_tostring(m_luaState, index));
	else if (type == LUA_TTABLE)
	{
		// Get associated native object.
		lua_rawgeti(m_luaState, index, c_tableKey_instance);
		if (lua_islightuserdata(m_luaState, -1))
		{
			Object* object = reinterpret_cast< Object* >(lua_touserdata(m_luaState, -1));
			lua_pop(m_luaState, 1);
			return Any::fromObject(object);
		}
		lua_pop(m_luaState, 1);

		// Unbox wrapped native type.
		lua_rawgeti(m_luaState, index, c_tableKey_class);
		if (lua_islightuserdata(m_luaState, -1))
		{
			IRuntimeClass* runtimeClass = reinterpret_cast< IRuntimeClass* >(lua_touserdata(m_luaState, -1));
			lua_pop(m_luaState, 1);
			if (runtimeClass)
				return Any::fromObject(new BoxedTypeInfo(runtimeClass->getExportType()));
		}
		lua_pop(m_luaState, 1);

		// Box LUA object into C++ container.
		lua_pushvalue(m_luaState, index);
		int32_t tableRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
		return Any::fromObject(new ScriptObjectLua(this, m_lockContext, m_luaState, tableRef));
	}
	else if (type == LUA_TFUNCTION)
	{
		// Box LUA function into C++ container.
		lua_pushvalue(m_luaState, index);
		return Any::fromObject(new ScriptDelegateLua(m_lockContext, m_luaState));
	}

	return Any();
}

void ScriptManagerLua::toAny(int32_t base, int32_t count, Any* outAnys)
{
	CHECK_LUA_STACK(m_luaState, 0);

	for (int32_t i = 0; i < count; ++i)
	{
		int32_t index = base + i;
		int32_t type = lua_type(m_luaState, index);

		if (type == LUA_TNUMBER)
		{
#if defined(T_LUA_5_2)
			if (lua_isinteger(m_luaState, index))
				outAnys[i] = Any::fromInt64(lua_tointeger(m_luaState, index));
			else
#endif
				outAnys[i] = Any::fromFloat(float(lua_tonumber(m_luaState, index)));
		}
		else if (type == LUA_TBOOLEAN)
			outAnys[i] = Any::fromBoolean(bool(lua_toboolean(m_luaState, index) != 0));
		else if (type == LUA_TSTRING)
			outAnys[i] = Any::fromString(lua_tostring(m_luaState, index));
		else if (type == LUA_TTABLE)
		{
			// Get associated native object.
			lua_rawgeti(m_luaState, index, c_tableKey_instance);
			if (lua_islightuserdata(m_luaState, -1))
			{
				Object* object = reinterpret_cast< Object* >(lua_touserdata(m_luaState, -1));
				lua_pop(m_luaState, 1);
				outAnys[i] = Any::fromObject(object);
				continue;
			}
			lua_pop(m_luaState, 1);

			// Unbox wrapped native type.
			lua_rawgeti(m_luaState, index, c_tableKey_class);
			if (lua_islightuserdata(m_luaState, -1))
			{
				IRuntimeClass* runtimeClass = reinterpret_cast< IRuntimeClass* >(lua_touserdata(m_luaState, -1));
				lua_pop(m_luaState, 1);
				if (runtimeClass)
				{
					outAnys[i] = Any::fromObject(new BoxedTypeInfo(runtimeClass->getExportType()));
					continue;
				}
			}
			lua_pop(m_luaState, 1);

			// Box LUA object into C++ container.
			lua_pushvalue(m_luaState, index);
			int32_t tableRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
			outAnys[i] = Any::fromObject(new ScriptObjectLua(this, m_lockContext, m_luaState, tableRef));
		}
		else if (type == LUA_TFUNCTION)
		{
			// Box LUA function into C++ container.
			lua_pushvalue(m_luaState, index);
			outAnys[i] = Any::fromObject(new ScriptDelegateLua(m_lockContext, m_luaState));
		}
	}
}

void ScriptManagerLua::collectGarbageFull()
{
#if defined(T_SCRIPT_LUA_USE_MT_LOCK)
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
#endif
	collectGarbageFullNoLock();
}

void ScriptManagerLua::collectGarbageFullNoLock()
{
	// Repeat GC until allocated memory doesn't decrease
	// further in ten consecutive GCs.
	int32_t count = 10;
	while (count > 0)
	{
		size_t memoryUseBefore = m_totalMemoryUse;
		lua_gc(m_luaState, LUA_GCCOLLECT, 0);
		
		if (m_totalMemoryUse < memoryUseBefore)
			count = 10;
		else
			count--;
	}
	m_lastMemoryUse = m_totalMemoryUse;
}

void ScriptManagerLua::collectGarbagePartial()
{
#if defined(T_LUA_5_2) && defined(T_SCRIPT_LUA_USE_GENERATIONAL_COLLECTOR)
#	if defined(T_SCRIPT_LUA_USE_MT_LOCK)
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
#	endif

	if (m_collectSteps < 0)
	{
		lua_gc(m_luaState, LUA_GCSTOP, 0);
		lua_gc(m_luaState, LUA_GCGEN, 0);
		m_collectSteps = 0;
	}

	T_ASSERT (lua_gc(m_luaState, LUA_GCISRUNNING, 0) == 0);

	m_collectTargetSteps += float(s_timer.getDeltaTime() * m_collectStepFrequency);

	int32_t targetSteps = int32_t(m_collectTargetSteps);
	while (m_collectSteps < targetSteps)
	{
		lua_gc(m_luaState, LUA_GCCOLLECT, 0);
		++m_collectSteps;
	}

#else

	float dT = min< float >(float(s_timer.getDeltaTime()), 0.1f);
	m_collectTargetSteps += dT * m_collectStepFrequency;

	int32_t targetSteps = int32_t(m_collectTargetSteps);
	if (m_collectSteps < targetSteps)
	{
#if defined(T_SCRIPT_LUA_USE_MT_LOCK)
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
#endif
		if (m_collectSteps < 0)
		{
			lua_gc(m_luaState, LUA_GCSTOP, 0);
			m_collectSteps = 0;
		}

#if defined(T_LUA_5_2)
		T_ASSERT (lua_gc(m_luaState, LUA_GCISRUNNING, 0) == 0);
#endif

		// Progress with garbage collector.
		while (m_collectSteps < targetSteps)
		{
			lua_gc(m_luaState, LUA_GCSTEP, 128);
			++m_collectSteps;
		}
		m_collectSteps = targetSteps;
	}

	if (m_lastMemoryUse <= 0)
		m_lastMemoryUse = m_totalMemoryUse;

	if (m_totalMemoryUse > m_lastMemoryUse)
	{
		// Calculate amount of garbage produced per second.
		float garbageProduced = (m_totalMemoryUse - m_lastMemoryUse) / dT;

		// Determine collector frequency from amount of garbage per second.
		m_collectStepFrequency = max< float >(
			clamp(garbageProduced / (64*1024), 1.0f, 60.0f),
			m_collectStepFrequency
		);
	}
	else if (m_totalMemoryUse < m_lastMemoryUse)
	{
		// Using less memory after this collection; slowly decrease
		// frequency until memory start to rise again.
		m_collectStepFrequency = max< float >(1.0f, m_collectStepFrequency - m_collectStepFrequency / 10.0f);
	}

	m_lastMemoryUse = m_totalMemoryUse;

#endif
}

void ScriptManagerLua::breakDebugger(lua_State* luaState)
{
	if (!m_debugger)
		return;

	lua_Debug ar = { 0 };
	lua_getstack(luaState, 1, &ar);
	lua_getinfo(luaState, "Snlu", &ar);

	m_debugger->actionBreak();
	m_debugger->analyzeState(luaState, &ar);
}

int ScriptManagerLua::classAlloc(lua_State* luaState)
{
	lua_newtable(luaState);
	return 1;
}

int ScriptManagerLua::classGc(lua_State* luaState)
{
	ITypedObject* object = toTypedObject(luaState, 1);
	if (object)
	{
		T_SAFE_ANONYMOUS_RELEASE(object);
	}
	return 0;
}

int ScriptManagerLua::classNew(lua_State* luaState)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT (manager);

	int32_t classId = (int32_t)lua_tointeger(luaState, lua_upvalueindex(2));
	const RegisteredClass& rc =	manager->m_classRegistry[classId];

	int32_t top = lua_gettop(luaState);

	Any argv[8];
	manager->toAny(2, top - 1, argv);

	// Discard all arguments, only instance table in stack.
	lua_settop(luaState, 1);

#if defined(T_HAVE_CAST_EXCEPTIONS)
	try
#endif
	{
		Ref< ITypedObject > object = rc.runtimeClass->construct(0, top - 1, argv);
		if (!object)
			return 0;
		
		lua_rawgeti(luaState, LUA_REGISTRYINDEX, rc.classTableRef);
		lua_setmetatable(luaState, -2);

		// Attach native object as light user value of table.
		lua_pushlightuserdata(luaState, (void*)object.ptr());
		lua_rawseti(luaState, -2, c_tableKey_instance);
		T_SAFE_ANONYMOUS_ADDREF(object);

		// Store object instance in weak table.
		lua_rawgeti(luaState, LUA_REGISTRYINDEX, manager->m_objectTableRef);
		lua_insert(luaState, -2);
		lua_rawseti(luaState, -2, int32_t(uint64_t(object.ptr())));
		lua_rawgeti(luaState, -1, int32_t(uint64_t(object.ptr())));
		lua_remove(luaState, -2);
		return 0;
	}
#if defined(T_HAVE_CAST_EXCEPTIONS)
	catch(const CastException& x)
	{
		log::error << L"Unhandled CastException occurred when calling constructor, class " << rc.runtimeClass->getExportType().getName() << L"; \"" << mbstows(x.what()) << L"\"." << Endl;
		manager->breakDebugger(luaState);
	}
#endif

	return 0;
}

int ScriptManagerLua::classCallUnknownMethod(lua_State* luaState)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT (manager);

	const IRuntimeClass* runtimeClass = reinterpret_cast< IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (runtimeClass);

	int32_t top = lua_gettop(luaState);
	if (top < 2)
		return 0;

	const char* methodName = lua_tostring(luaState, 1);
	T_ASSERT (methodName);

	ITypedObject* object = toTypedObject(luaState, 2);
	if (!object)
	{
		log::error << L"Unable to call method \"" << mbstows(methodName) << L"\", class " << runtimeClass->getExportType().getName() << L"; null object" << Endl;
		return 0;
	}

	Any argv[8];
	manager->toAny(3, top - 2, argv);

#if defined(T_HAVE_CAST_EXCEPTIONS)
	try
#endif
	{
		Any returnValue = runtimeClass->invokeUnknown(object, methodName, top - 2, argv);
		manager->pushAny(returnValue);
		return 1;
	}
#if defined(T_HAVE_CAST_EXCEPTIONS)
	catch(const CastException& x)
	{
		log::error << L"Unhandled CastException occurred when calling unknown method \"" << mbstows(methodName) << L"\", class " << runtimeClass->getExportType().getName() << L"; \"" << mbstows(x.what()) << L"\"." << Endl;
		manager->breakDebugger(luaState);
	}
#endif

	return 0;
}

int ScriptManagerLua::classCallMethod(lua_State* luaState)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (manager);

	const IRuntimeClass* runtimeClass = reinterpret_cast< IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(3)));
	T_ASSERT (runtimeClass);

	int32_t methodId = (int32_t)lua_tonumber(luaState, lua_upvalueindex(1));

	int32_t top = lua_gettop(luaState);
	if (top < 1)
		return 0;

	ITypedObject* object = toTypedObject(luaState, 1);
	if (!object)
	{
		log::error << L"Unable to call method \"" << mbstows(runtimeClass->getMethodName(methodId)) << L"\"; null object" << Endl;
		return 0;
	}

	Any argv[10];
	manager->toAny(2, top - 1, argv);

#if defined(T_HAVE_CAST_EXCEPTIONS)
	try
#endif
	{
		Any returnValue = runtimeClass->invoke(object, methodId, top - 1, argv);
		manager->pushAny(returnValue);
		return 1;
	}
#if defined(T_HAVE_CAST_EXCEPTIONS)
	catch(const CastException& x)
	{
		log::error << L"Unhandled CastException occurred when calling method \"" << mbstows(runtimeClass->getMethodName(methodId)) << L"\", class " << runtimeClass->getExportType().getName() << L"; \"" << mbstows(x.what()) << L"\"." << Endl;
		manager->breakDebugger(luaState);
	}
#endif

	return 0;
}

int ScriptManagerLua::classCallStaticMethod(lua_State* luaState)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (manager);

	const IRuntimeClass* runtimeClass = reinterpret_cast< IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(3)));
	T_ASSERT (runtimeClass);

	int32_t methodId = (int32_t)lua_tonumber(luaState, lua_upvalueindex(1));

	int32_t top = lua_gettop(luaState);
	if (top < 0)
		return 0;

	Any argv[10];
	manager->toAny(1, top, argv);

#if defined(T_HAVE_CAST_EXCEPTIONS)
	try
#endif
	{
		Any returnValue = runtimeClass->invokeStatic(methodId, top, argv);
		manager->pushAny(returnValue);
		return 1;
	}
#if defined(T_HAVE_CAST_EXCEPTIONS)
	catch(const CastException& x)
	{
		log::error << L"Unhandled CastException occurred when calling static method \"" << mbstows(runtimeClass->getStaticMethodName(methodId)) << L"\", class " << runtimeClass->getExportType().getName() << L"; \"" << mbstows(x.what()) << L"\"." << Endl;
		manager->breakDebugger(luaState);
	}
#endif

	return 0;
}

int ScriptManagerLua::classSetProperty(lua_State* luaState)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (manager);

	const IRuntimeClass* runtimeClass = reinterpret_cast< IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(3)));
	T_ASSERT (runtimeClass);

	int32_t propertyId = (int32_t)lua_tonumber(luaState, lua_upvalueindex(1));

	ITypedObject* object = toTypedObject(luaState, 1);
	if (!object)
	{
		log::error << L"Unable to set property \"" << mbstows(runtimeClass->getPropertyName(propertyId)) << L"\"; null object" << Endl;
		return 0;
	}

	Any value = manager->toAny(2);
	runtimeClass->invokePropertySet(object, propertyId, value);

	return 0;
}

int ScriptManagerLua::classGetProperty(lua_State* luaState)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (manager);

	const IRuntimeClass* runtimeClass = reinterpret_cast< IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(3)));
	T_ASSERT (runtimeClass);

	int32_t propertyId = (int32_t)lua_tonumber(luaState, lua_upvalueindex(1));

	ITypedObject* object = toTypedObject(luaState, 1);
	if (!object)
	{
		log::error << L"Unable to get property \"" << mbstows(runtimeClass->getPropertyName(propertyId)) << L"\"; null object" << Endl;
		return 0;
	}

	Any value = runtimeClass->invokePropertyGet(object, propertyId);
	manager->pushAny(value);

	return 1;
}

int ScriptManagerLua::classEqual(lua_State* luaState)
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

int ScriptManagerLua::classAdd(lua_State* luaState)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT (manager);

	const IRuntimeClass* runtimeClass = reinterpret_cast< IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (runtimeClass);

	int32_t top = lua_gettop(luaState);
	if (top < 1)
		return 0;

	ITypedObject* object = 0;
	Any arg;

	if (lua_istable(luaState, 1))
	{
		object = toTypedObject(luaState, 1);
		arg = manager->toAny(2);
	}
	else if (lua_isuserdata(luaState, 2))
	{
		object = toTypedObject(luaState, 2);
		arg = manager->toAny(1);
	}

	if (!object)
	{
		log::error << L"Unable to call add operator, class " << runtimeClass->getExportType().getName() << L"; null object" << Endl;
		return 0;
	}

#if defined(T_HAVE_CAST_EXCEPTIONS)
	try
#endif
	{
		Any returnValue = runtimeClass->invokeOperator(object, 0, arg);
		manager->pushAny(returnValue);
		return 1;
	}
#if defined(T_HAVE_CAST_EXCEPTIONS)
	catch(const CastException& x)
	{
		log::error << L"Unhandled CastException occurred when calling add operator, class " << runtimeClass->getExportType().getName() << L"; \"" << mbstows(x.what()) << L"\"." << Endl;
		manager->breakDebugger(luaState);
	}
#endif

	return 0;
}

int ScriptManagerLua::classSubtract(lua_State* luaState)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT (manager);

	const IRuntimeClass* runtimeClass = reinterpret_cast< IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (runtimeClass);

	int32_t top = lua_gettop(luaState);
	if (top < 1)
		return 0;

	ITypedObject* object = toTypedObject(luaState, 1);
	if (!object)
	{
		log::error << L"Unable to call subtract operator, class " << runtimeClass->getExportType().getName() << L"; null object" << Endl;
		return 0;
	}

	Any arg = manager->toAny(2);

#if defined(T_HAVE_CAST_EXCEPTIONS)
	try
#endif
	{
		Any returnValue = runtimeClass->invokeOperator(object, 1, arg);
		manager->pushAny(returnValue);
		return 1;
	}
#if defined(T_HAVE_CAST_EXCEPTIONS)
	catch(const CastException& x)
	{
		log::error << L"Unhandled CastException occurred when calling subtract operator, class " << runtimeClass->getExportType().getName() << L"; \"" << mbstows(x.what()) << L"\"." << Endl;
		manager->breakDebugger(luaState);
	}
#endif

	return 0;
}

int ScriptManagerLua::classMultiply(lua_State* luaState)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT (manager);

	const IRuntimeClass* runtimeClass = reinterpret_cast< IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (runtimeClass);

	int32_t top = lua_gettop(luaState);
	if (top < 1)
		return 0;

	ITypedObject* object = 0;
	Any arg;

	if (lua_istable(luaState, 1))
	{
		object = toTypedObject(luaState, 1);
		arg = manager->toAny(2);
	}
	else if (lua_isuserdata(luaState, 2))
	{
		object = toTypedObject(luaState, 2);
		arg = manager->toAny(1);
	}

	if (!object)
	{
		log::error << L"Unable to call multiply operator, class " << runtimeClass->getExportType().getName() << L"; null object" << Endl;
		return 0;
	}

#if defined(T_HAVE_CAST_EXCEPTIONS)
	try
#endif
	{
		Any returnValue = runtimeClass->invokeOperator(object, 2, arg);
		manager->pushAny(returnValue);
		return 1;
	}
#if defined(T_HAVE_CAST_EXCEPTIONS)
	catch(const CastException& x)
	{
		log::error << L"Unhandled CastException occurred when calling multiply operator, class " << runtimeClass->getExportType().getName() << L"; \"" << mbstows(x.what()) << L"\"." << Endl;
		manager->breakDebugger(luaState);
	}
#endif

	return 0;
}

int ScriptManagerLua::classDivide(lua_State* luaState)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT (manager);

	const IRuntimeClass* runtimeClass = reinterpret_cast< IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (runtimeClass);

	int32_t top = lua_gettop(luaState);
	if (top < 1)
		return 0;

	ITypedObject* object = toTypedObject(luaState, 1);
	if (!object)
	{
		log::error << L"Unable to call divide operator, class " << runtimeClass->getExportType().getName() << L"; null object" << Endl;
		return 0;
	}

	Any arg = manager->toAny(2);

#if defined(T_HAVE_CAST_EXCEPTIONS)
	try
#endif
	{
		Any returnValue = runtimeClass->invokeOperator(object, 3, arg);
		manager->pushAny(returnValue);
		return 1;
	}
#if defined(T_HAVE_CAST_EXCEPTIONS)
	catch(const CastException& x)
	{
		log::error << L"Unhandled CastException occurred when calling divide operator, class " << runtimeClass->getExportType().getName() << L"; \"" << mbstows(x.what()) << L"\"." << Endl;
		manager->breakDebugger(luaState);
	}
#endif

	return 0;
}

void* ScriptManagerLua::luaAlloc(void* ud, void* ptr, size_t osize, size_t nsize)
{
	ScriptManagerLua* this_ = reinterpret_cast< ScriptManagerLua* >(ud);
	T_ASSERT (this_);

	IAllocator* allocator = getAllocator();
	size_t& totalMemoryUse = this_->m_totalMemoryUse;
	if (nsize > 0)
	{
		totalMemoryUse += nsize;

#if defined(T_USE_ALLOCATOR)
		if (osize >= nsize && osize - nsize < 512)
		{
			T_ASSERT (ptr);
			totalMemoryUse -= osize;
			return ptr;
		}

		void* nptr = allocator->alloc(nsize, 16, T_FILE_LINE);
		if (!nptr)
			return 0;
#endif

		if (ptr && osize > 0)
		{
#if defined(T_USE_ALLOCATOR)
			std::memcpy(nptr, ptr, std::min(osize, nsize));
			allocator->free(ptr);
#endif
			T_ASSERT (osize <= totalMemoryUse);
			totalMemoryUse -= osize;
		}

#if defined(T_USE_ALLOCATOR)
		return nptr;
#endif
	}
	else if (ptr)
	{
#if defined(T_USE_ALLOCATOR)
		allocator->free(ptr);
#endif
		T_ASSERT (osize <= totalMemoryUse);
		totalMemoryUse -= osize;
	}

#if defined(T_USE_ALLOCATOR)
	return 0;
#else
	return ((lua_Alloc)(this_->m_defaultAllocFn))(this_->m_defaultAllocOpaque, ptr, osize, nsize);
#endif
}

int ScriptManagerLua::luaAllocatedMemory(lua_State* luaState)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(G(luaState)->ud);
	lua_pushinteger(luaState, lua_Integer(manager->m_totalMemoryUse));
	return 1;
}

void ScriptManagerLua::hookCallback(lua_State* luaState, lua_Debug* ar)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(G(luaState)->ud);
	if (manager->m_debugger)
		manager->m_debugger->hookCallback(luaState, ar);
	if (manager->m_profiler)
		manager->m_profiler->hookCallback(luaState, ar);
}

int ScriptManagerLua::luaPanic(lua_State* luaState)
{
	log::error << L"LUA PANIC; Unrecoverable error \"" << mbstows(lua_tostring(luaState, lua_gettop(luaState))) << L"\"" << Endl;
	return 0;
}

	}
}
