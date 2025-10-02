/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Class/AutoVerify.h"
#include "Core/Class/Boxes/BoxedTypeInfo.h"
#include "Core/Class/IRuntimeClass.h"
#include "Core/Class/IRuntimeDispatch.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/Save.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Thread/Acquire.h"
#include "Core/Timer/Timer.h"
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

#define T_USE_ALLOCATOR 1
#define T_LOG_OBJECT_GC 0

namespace traktor::script
{
	namespace
	{

Timer s_timer;

const int32_t c_tableKey_class = -1;
const int32_t c_tableKey_instance = -2;

inline ITypedObject* toTypedObject(lua_State* luaState, int32_t index)
{
	lua_rawgeti(luaState, index, c_tableKey_instance);
	if (!lua_islightuserdata(luaState, -1))
	{
		lua_pop(luaState, 1);
		return nullptr;
	}

	ITypedObject* object = (ITypedObject*)lua_touserdata(luaState, -1);
	if (!object)
	{
		lua_pop(luaState, 1);
		return nullptr;
	}

	lua_pop(luaState, 1);
	return object;
}

inline void getObjectRef(lua_State* L, int32_t objectTableRef, ITypedObject* object)
{
	CHECK_LUA_STACK(L, 1);
	lua_rawgeti(L, LUA_REGISTRYINDEX, objectTableRef);
	lua_pushinteger(L, lua_Integer(object));
	lua_rawget(L, -2);
	lua_remove(L, -2);
}

inline void putObjectRef(lua_State* L, int32_t objectTableRef, ITypedObject* object)
{
	CHECK_LUA_STACK(L, 0);
	lua_rawgeti(L, LUA_REGISTRYINDEX, objectTableRef);
	lua_pushinteger(L, lua_Integer(object));
	lua_pushvalue(L, -3);
	lua_rawset(L, -3);
	lua_pop(L, 1);
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptManagerLua", 0, ScriptManagerLua, IScriptManager)

ScriptManagerLua* ScriptManagerLua::ms_instance = nullptr;

ScriptManagerLua::ScriptManagerLua()
:	m_luaState(nullptr)
,	m_defaultAllocFn(nullptr)
,	m_defaultAllocOpaque(nullptr)
,	m_lockContext(nullptr)
,	m_collectStepFrequency(10.0)
,	m_collectSteps(-1)
,	m_collectTargetSteps(0.0f)
,	m_totalMemoryUse(0)
,	m_lastMemoryUse(0)
{
	T_FATAL_ASSERT(ms_instance == nullptr);
	ms_instance = this;

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
	luaL_openlibs(m_luaState);

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

#if defined(_DEBUG)
		lua_pushstring(m_luaState, "native instance ref table");
		lua_setfield(m_luaState, -2, "__name");
#endif

		m_objectTableRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_objectTableRef);

		lua_newtable(m_luaState);
		lua_pushstring(m_luaState, "kv");
		lua_setfield(m_luaState, -2, "__mode");
		lua_setmetatable(m_luaState, -2);

		lua_pop(m_luaState, 1);
	}

	s_timer.reset();
}

ScriptManagerLua::~ScriptManagerLua()
{
	T_FATAL_ASSERT_M(!m_luaState, L"Must call destroy");
	T_FATAL_ASSERT(ms_instance == this);
	ms_instance = nullptr;
}

void ScriptManagerLua::destroy()
{
	if (!m_luaState)
		return;

	T_ANONYMOUS_VAR(Ref< ScriptManagerLua >)(this);
	T_FATAL_ASSERT(m_contexts.empty());

	// Discard member first since ScriptObjectLua check if
	// state is valid when destroying and since we're already
	// in the process of shutting down the state we don't
	// want the objects to interfere with us.
	lua_State* luaState = m_luaState;
	m_luaState = nullptr;

	// Discard all tags from C++ rtti types.
	for (auto& rc : m_classRegistry)
	{
		for (auto& derivedType : rc.runtimeClass->getExportType().findAllOf())
			derivedType->setTag(0);
	}

	m_debugger = nullptr;
	m_profiler = nullptr;

	luaL_unref(luaState, LUA_REGISTRYINDEX, m_objectTableRef);
	m_objectTableRef = LUA_NOREF;

	lua_close(luaState);
}

void ScriptManagerLua::registerClass(IRuntimeClass* runtimeClass)
{
	T_ANONYMOUS_VAR(UnwindStack)(m_luaState);

	const TypeInfo& exportType = runtimeClass->getExportType();
	const int32_t classRegistryIndex = int32_t(m_classRegistry.size());

	RegisteredClass& rc = m_classRegistry.push_back();
	rc.runtimeClass = runtimeClass;

	// Create new class.
	lua_getglobal(m_luaState, "class");
	T_FATAL_ASSERT (lua_isfunction(m_luaState, -1));
	lua_pushstring(m_luaState, wstombs(exportType.getName()).c_str());
	if (exportType.getSuper())
	{
		const int32_t superClassId = int32_t(exportType.getSuper()->getTag()) - 1;
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

	// Create "__gc" callback to be able to track C++ object lifetime.
	lua_pushcfunction(m_luaState, classGc);
	lua_setfield(m_luaState, -2, "__gc");

	// Create "new" callback to be able to instantiate C++ object when creating from script side.
	if (runtimeClass->getConstructorDispatch())
	{
		lua_pushlightuserdata(m_luaState, (void*)runtimeClass->getConstructorDispatch());
		lua_pushinteger(m_luaState, classRegistryIndex);
		lua_pushcclosure(m_luaState, classNew, 2);
		lua_setfield(m_luaState, -2, "new");
	}

	// Add static methods.
	const uint32_t staticMethodCount = runtimeClass->getStaticMethodCount();
	for (uint32_t i = 0; i < staticMethodCount; ++i)
	{
		const std::string methodName = runtimeClass->getStaticMethodName(i);
		lua_pushlightuserdata(m_luaState, (void*)runtimeClass->getStaticMethodDispatch(i));
		lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
		lua_pushcclosure(m_luaState, classCallStaticMethod, 2);
		lua_setfield(m_luaState, -2, methodName.c_str());
	}

	// Add methods.
	const uint32_t methodCount = runtimeClass->getMethodCount();
	for (uint32_t i = 0; i < methodCount; ++i)
	{
		const std::string methodName = runtimeClass->getMethodName(i);
		lua_pushlightuserdata(m_luaState, (void*)runtimeClass->getMethodDispatch(i));
		lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
		lua_pushcclosure(m_luaState, classCallMethod, 2);
		lua_setfield(m_luaState, -2, methodName.c_str());
	}

	// Add properties.
	T_FATAL_ASSERT (lua_istable(m_luaState, - 1));
	const uint32_t propertyCount = runtimeClass->getPropertiesCount();
	for (uint32_t i = 0; i < propertyCount; ++i)
	{
		const std::string propertyName = runtimeClass->getPropertyName(i);

		lua_getfield(m_luaState, -1, "__setters");
		T_FATAL_ASSERT(lua_istable(m_luaState, - 1));

		lua_pushlightuserdata(m_luaState, (void*)runtimeClass->getPropertySetDispatch(i));
		lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
		lua_pushcclosure(m_luaState, classSetProperty, 2);
		T_FATAL_ASSERT(lua_isfunction(m_luaState, - 1));

		lua_setfield(m_luaState, -2, propertyName.c_str());
		lua_pop(m_luaState, 1);

		lua_getfield(m_luaState, -1, "__getters");
		T_FATAL_ASSERT(lua_istable(m_luaState, - 1));

		lua_pushlightuserdata(m_luaState, (void*)runtimeClass->getPropertyGetDispatch(i));
		lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
		lua_pushcclosure(m_luaState, classGetProperty, 2);
		T_FATAL_ASSERT(lua_isfunction(m_luaState, - 1));

		lua_setfield(m_luaState, -2, propertyName.c_str());
		lua_pop(m_luaState, 1);
	}

	// Add operators.
	lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
	lua_pushcclosure(m_luaState, classEqual, 1);
	lua_setfield(m_luaState, -2, "__eq");

	{
		const IRuntimeDispatch* addDispatch = runtimeClass->getOperatorDispatch(IRuntimeClass::Operator::Add);
		if (addDispatch)
		{
			lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
			lua_pushlightuserdata(m_luaState, (void*)addDispatch);
			lua_pushcclosure(m_luaState, classAdd, 2);
			lua_setfield(m_luaState, -2, "__add");
		}
	}

	{
		const IRuntimeDispatch* subDispatch = runtimeClass->getOperatorDispatch(IRuntimeClass::Operator::Subtract);
		if (subDispatch)
		{
			lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
			lua_pushlightuserdata(m_luaState, (void*)subDispatch);
			lua_pushcclosure(m_luaState, classSubtract, 2);
			lua_setfield(m_luaState, -2, "__sub");
		}
	}

	{
		const IRuntimeDispatch* mulDispatch = runtimeClass->getOperatorDispatch(IRuntimeClass::Operator::Multiply);
		if (mulDispatch)
		{
			lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
			lua_pushlightuserdata(m_luaState, (void*)mulDispatch);
			lua_pushcclosure(m_luaState, classMultiply, 2);
			lua_setfield(m_luaState, -2, "__mul");
		}
	}

	{
		const IRuntimeDispatch* divDispatch = runtimeClass->getOperatorDispatch(IRuntimeClass::Operator::Divide);
		if (divDispatch)
		{
			lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
			lua_pushlightuserdata(m_luaState, (void*)divDispatch);
			lua_pushcclosure(m_luaState, classDivide, 2);
			lua_setfield(m_luaState, -2, "__div");
		}
	}

	rc.classTableRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);

	// __newindex
	{
		DO_0(m_luaState, lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, rc.classTableRef)	);
		DO_1(m_luaState, lua_getfield(m_luaState, -1, "__setters")						);
		DO_1(m_luaState, lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, rc.classTableRef)	);
		DO_1(m_luaState, lua_pushcclosure(m_luaState, classNewIndex, 2)					);
		DO_1(m_luaState, lua_setfield(m_luaState, -2, "__newindex")						);
	}

	// __index
	{
		DO_0(m_luaState, lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, rc.classTableRef)	);
		DO_1(m_luaState, lua_getfield(m_luaState, -1, "__getters")						);
		DO_1(m_luaState, lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, rc.classTableRef)	);
		DO_1(m_luaState, lua_pushcclosure(m_luaState, classIndex, 2)					);
		DO_1(m_luaState, lua_setfield(m_luaState, -2, "__index")						);
	}

	// Export class in global scope.
	std::wstring exportName = exportType.getName();
	std::vector< std::wstring > exportPath;
	Split< std::wstring >::any(exportName, L".", exportPath);

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
				T_ASSERT(lua_istable(m_luaState, -1));
			}
			else
				lua_replace(m_luaState, -2);
		}
	}

	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, rc.classTableRef);
	lua_setfield(m_luaState, -2, wstombs(exportPath.back()).c_str());

	lua_pop(m_luaState, 1);

	// Store index of registered script class in C++ rtti type; used
	// to accelerate lookup of C++ class when constructing new instance from script.
	// Need to propagate index into derived types as well in order to
	// be able to skip traversing class hierarchy while constructing.
	for (auto derivedType : exportType.findAllOf())
	{
		if (derivedType->getTag() != 0)
		{
			const RegisteredClass& rc2 = m_classRegistry[derivedType->getTag() - 1];
			const TypeInfo& exportType2 = rc2.runtimeClass->getExportType();
			if (is_type_of(exportType, exportType2))
				continue;
		}
		derivedType->setTag(classRegistryIndex + 1);
	}

	// Add constants last as constants might be instances of this class, i.e. singletons etc.
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, rc.classTableRef);
	for (uint32_t i = 0; i < runtimeClass->getConstantCount(); ++i)
	{
		pushAny(runtimeClass->getConstantValue(i));
		lua_setfield(m_luaState, -2, runtimeClass->getConstantName(i).c_str());
	}
}

void ScriptManagerLua::completeRegistration()
{
	// Lua doesn't need two-phase registration; everything is done in registerClass().
}

Ref< IScriptContext > ScriptManagerLua::createContext(bool strict)
{
#if defined(T_SCRIPT_LUA_USE_MT_LOCK)
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
#endif
	CHECK_LUA_STACK(m_luaState, 0);

	// Create local environment table and add to registry.
	lua_newtable(m_luaState);
	const int32_t environmentRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
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
		const ScriptObjectLua* scriptObject = static_cast< const ScriptObjectLua* >(object);
		scriptObject->push();
		return;
	}
	else if (&objectType == &type_of< ScriptDelegateLua >())
	{
		const ScriptDelegateLua* delegateContainer = static_cast< const ScriptDelegateLua* >(object);
		delegateContainer->push();
		return;
	}

	// Get cached script-land table of this instance.
	getObjectRef(m_luaState, m_objectTableRef, object);
	if (lua_istable(m_luaState, -1))
		return;
	lua_pop(m_luaState, 1);

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

#if defined(_DEBUG)
	lua_pushstring(m_luaState, "native instance");
	lua_setfield(m_luaState, -2, "__name");
	lua_pushstring(m_luaState, wstombs(objectType.getName()).c_str());
	lua_setfield(m_luaState, -2, "__typename");
#endif

	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, rc.classTableRef);
	lua_setmetatable(m_luaState, -2);

	// Attach native object as light user value of table.
	lua_pushlightuserdata(m_luaState, (void*)object);
	lua_rawseti(m_luaState, -2, c_tableKey_instance);
	T_SAFE_ADDREF(object);

	// Store object instance in weak table.
	putObjectRef(m_luaState, m_objectTableRef, object);
}

void ScriptManagerLua::pushAny(const Any& any)
{
	CHECK_LUA_STACK(m_luaState, 1);
	switch (any.getType())
	{
	case Any::Type::Boolean:
		lua_pushboolean(m_luaState, any.getBooleanUnsafe() ? 1 : 0);
		break;
	case Any::Type::Int32:
		lua_pushinteger(m_luaState, any.getInt32Unsafe());
		break;
	case Any::Type::Int64:
		lua_pushinteger(m_luaState, any.getInt64Unsafe());
		break;
	case Any::Type::Float:
		lua_pushnumber(m_luaState, any.getFloatUnsafe());
		break;
	case Any::Type::Double:
		lua_pushnumber(m_luaState, any.getDoubleUnsafe());
		break;
	case Any::Type::String:
		lua_pushstring(m_luaState, any.getCStringUnsafe());
		break;
	case Any::Type::Object:
		pushObject(any.getObjectUnsafe());
		break;
	default:
		lua_pushnil(m_luaState);
		break;
	}
}

void ScriptManagerLua::pushAny(const Any* anys, int32_t count)
{
	CHECK_LUA_STACK(m_luaState, count);
	for (int32_t i = 0; i < count; ++i)
	{
		const Any& any = anys[i];
		switch (any.getType())
		{
		case Any::Type::Boolean:
			lua_pushboolean(m_luaState, any.getBooleanUnsafe() ? 1 : 0);
			break;
		case Any::Type::Int32:
			lua_pushinteger(m_luaState, any.getInt32Unsafe());
			break;
		case Any::Type::Int64:
			lua_pushinteger(m_luaState, any.getInt64Unsafe());
			break;
		case Any::Type::Float:
			lua_pushnumber(m_luaState, any.getFloatUnsafe());
			break;
		case Any::Type::Double:
			lua_pushnumber(m_luaState, any.getDoubleUnsafe());
			break;
		case Any::Type::String:
			lua_pushstring(m_luaState, any.getCStringUnsafe());
			break;
		case Any::Type::Object:
			pushObject(any.getObjectUnsafe());
			break;
		default:
			lua_pushnil(m_luaState);
			break;
		}
	}
}

Any ScriptManagerLua::toAny(int32_t index)
{
	CHECK_LUA_STACK(m_luaState, 0);

	const int32_t type = lua_type(m_luaState, index);
	switch (type)
	{
	case LUA_TNUMBER:
		{
			if (lua_isinteger(m_luaState, index))
				return Any::fromInt64(lua_tointeger(m_luaState, index));
			else
				return Any::fromDouble(lua_tonumber(m_luaState, index));
		}
	case LUA_TBOOLEAN:
		return Any::fromBoolean(bool(lua_toboolean(m_luaState, index) != 0));
	case LUA_TSTRING:
		return Any::fromString(lua_tostring(m_luaState, index));
	case LUA_TTABLE:
		{
			// Get associated native object.
			lua_rawgeti(m_luaState, index, c_tableKey_instance);
			if (lua_islightuserdata(m_luaState, -1))
			{
				Object* object = reinterpret_cast<Object*>(lua_touserdata(m_luaState, -1));
				lua_pop(m_luaState, 1);
				return Any::fromObject(object);
			}
			lua_pop(m_luaState, 1);

			// Unbox wrapped native type.
			lua_rawgeti(m_luaState, index, c_tableKey_class);
			if (lua_islightuserdata(m_luaState, -1))
			{
				IRuntimeClass* runtimeClass = reinterpret_cast<IRuntimeClass*>(lua_touserdata(m_luaState, -1));
				lua_pop(m_luaState, 1);
				if (runtimeClass)
					return Any::fromObject(new BoxedTypeInfo(runtimeClass->getExportType()));
			}
			lua_pop(m_luaState, 1);

			// Box LUA object into C++ container.
			lua_pushvalue(m_luaState, index);
			const int32_t tableRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
			return Any::fromObject(new ScriptObjectLua(this, m_lockContext, m_luaState, tableRef));
		}
	case LUA_TFUNCTION:
		{
			// Box LUA function into C++ container.
			lua_pushvalue(m_luaState, index);
			const int32_t functionRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
			return Any::fromObject(new ScriptDelegateLua(m_lockContext, m_luaState, functionRef));
		}
	default:
		break;
	}
	return Any();
}

void ScriptManagerLua::toAny(int32_t base, int32_t count, Any* outAnys)
{
	CHECK_LUA_STACK(m_luaState, 0);

	for (int32_t i = 0; i < count; ++i)
	{
		const int32_t index = base + i;
		const int32_t type = lua_type(m_luaState, index);

		switch (type)
		{
		case LUA_TNUMBER:
			{
				if (lua_isinteger(m_luaState, index))
					outAnys[i] = Any::fromInt64(lua_tointeger(m_luaState, index));
				else
					outAnys[i] = Any::fromDouble(lua_tonumber(m_luaState, index));
			}
			break;
		case LUA_TBOOLEAN:
			outAnys[i] = Any::fromBoolean(bool(lua_toboolean(m_luaState, index) != 0));
			break;
		case LUA_TSTRING:
			outAnys[i] = Any::fromString(lua_tostring(m_luaState, index));
			break;
		case LUA_TTABLE:
			{
				// Get associated native object.
				lua_rawgeti(m_luaState, index, c_tableKey_instance);
				if (lua_islightuserdata(m_luaState, -1))
				{
					Object* object = reinterpret_cast<Object*>(lua_touserdata(m_luaState, -1));
					lua_pop(m_luaState, 1);
					outAnys[i] = Any::fromObject(object);
					continue;
				}
				lua_pop(m_luaState, 1);

				// Unbox wrapped native type.
				lua_rawgeti(m_luaState, index, c_tableKey_class);
				if (lua_islightuserdata(m_luaState, -1))
				{
					IRuntimeClass* runtimeClass = reinterpret_cast<IRuntimeClass*>(lua_touserdata(m_luaState, -1));
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
				const int32_t tableRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
				outAnys[i] = Any::fromObject(new ScriptObjectLua(this, m_lockContext, m_luaState, tableRef));
			}
			break;
		case LUA_TFUNCTION:
			{
				// Box LUA function into C++ container.
				lua_pushvalue(m_luaState, index);
				const int32_t functionRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
				outAnys[i] = Any::fromObject(new ScriptDelegateLua(m_lockContext, m_luaState, functionRef));
			}
			break;
		default:
			break;
		}
	}
}

void ScriptManagerLua::destroyContext(ScriptContextLua* context)
{
	m_contexts.remove(context);
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
	// further in multiple consecutive GCs.
	int32_t count = 100;
	while (count > 0)
	{
		const size_t memoryUseBefore = m_totalMemoryUse;
		lua_gc(m_luaState, LUA_GCCOLLECT, 0);

		if (m_totalMemoryUse < memoryUseBefore)
			count = 100;
		else
			count--;
	}
	m_lastMemoryUse = m_totalMemoryUse;
}

void ScriptManagerLua::collectGarbagePartial()
{
#if defined(T_SCRIPT_LUA_USE_GENERATIONAL_COLLECTOR)
#	if defined(T_SCRIPT_LUA_USE_MT_LOCK)
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
#	endif

	if (m_collectSteps < 0)
	{
		lua_gc(m_luaState, LUA_GCSTOP, 0);
		lua_gc(m_luaState, LUA_GCGEN, 0);
		m_collectSteps = 0;
	}

	T_ASSERT(lua_gc(m_luaState, LUA_GCISRUNNING, 0) == 0);

	m_collectTargetSteps += float(s_timer.getDeltaTime() * m_collectStepFrequency);

	int32_t targetSteps = int32_t(m_collectTargetSteps);
	while (m_collectSteps < targetSteps)
	{
		lua_gc(m_luaState, LUA_GCCOLLECT, 0);
		++m_collectSteps;
	}

#else

	const float dT = std::min< float >((float)s_timer.getDeltaTime(), 0.1f);
	m_collectTargetSteps += dT * m_collectStepFrequency;

	const int32_t targetSteps = int32_t(m_collectTargetSteps);
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

		T_ASSERT(lua_gc(m_luaState, LUA_GCISRUNNING, 0) == 0);

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
		const float garbageProduced = (m_totalMemoryUse - m_lastMemoryUse) / dT;

		// Determine collector frequency from amount of garbage per second.
		m_collectStepFrequency = std::max< float >(
			clamp(garbageProduced / (64*1024), 1.0f, 60.0f),
			m_collectStepFrequency
		);
	}
	else if (m_totalMemoryUse < m_lastMemoryUse)
	{
		// Using less memory after this collection; slowly decrease
		// frequency until memory start to rise again.
		m_collectStepFrequency = std::max< float >(1.0f, m_collectStepFrequency - m_collectStepFrequency / 10.0f);
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

int ScriptManagerLua::classGc(lua_State* luaState)
{
#if T_LOG_OBJECT_GC
	std::wstring typeName = getObjectTypeName(luaState, 1);
	if (!typeName.empty())
		log::debug << L"ScriptManagerLua::classGC; releasing \"" << typeName << L"\"." << Endl;
#endif

	ITypedObject* object = toTypedObject(luaState, 1);
	if (object) [[likely]]
	{
#if T_LOG_OBJECT_GC
		if (typeName.empty())
			log::debug << L"ScriptManagerLua::classGC; releasing object without \"__typename\" tag, would be \"" << type_name(object) << L"\"." << Endl;
#endif
		T_SAFE_ANONYMOUS_RELEASE(object);
	}
	return 0;
}

int ScriptManagerLua::classNew(lua_State* luaState)
{
	const int32_t classId = (int32_t)lua_tointeger(luaState, lua_upvalueindex(2));
	const RegisteredClass& rc =	ms_instance->m_classRegistry[classId];

	const IRuntimeDispatch* runtimeDispatch = reinterpret_cast< const IRuntimeDispatch* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT(runtimeDispatch);

	const int32_t top = lua_gettop(luaState);

	Any argv[8];
	ms_instance->toAny(2, top - 1, argv);

	// Discard all arguments, only instance table in stack.
	lua_settop(luaState, 1);

#if T_VERIFY_USING_EXCEPTIONS
	try
#endif
	{
		Ref< ITypedObject > object = runtimeDispatch->invoke(0, top - 1, argv).getObject();
		if (!object) [[unlikely]]
			return 0;

		lua_rawgeti(luaState, LUA_REGISTRYINDEX, rc.classTableRef);
		lua_setmetatable(luaState, -2);

		// Attach native object as light user value of table.
		lua_pushlightuserdata(luaState, (void*)object.ptr());
		lua_rawseti(luaState, -2, c_tableKey_instance);
		T_SAFE_ANONYMOUS_ADDREF(object);

#if defined(_DEBUG)
		const TypeInfo& objectType = type_of(object);
		lua_pushstring(luaState, wstombs(objectType.getName()).c_str());
		lua_setfield(luaState, -2, "__typename");
#endif

		// Store object instance in weak table.
		putObjectRef(luaState, ms_instance->m_objectTableRef, object);
		return 1;
	}
#if T_VERIFY_USING_EXCEPTIONS
	catch(const RuntimeException& x)
	{
		log::error << L"Unhandled RuntimeException occurred when calling constructor, class " << rc.runtimeClass->getExportType().getName() << L"; \"" << x.what() << L"\"." << Endl;
		ms_instance->breakDebugger(luaState);
	}
#endif

	return 0;
}

int ScriptManagerLua::classCallUnknownMethod(lua_State* luaState)
{
	const IRuntimeClass* runtimeClass = reinterpret_cast< const IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT(runtimeClass);

	const IRuntimeDispatch* runtimeDispatch = reinterpret_cast< const IRuntimeDispatch* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT(runtimeDispatch);

	const int32_t top = lua_gettop(luaState);
	if (top < 2) [[unlikely]]
		return 0;

	const char* methodName = lua_tostring(luaState, 1);
	T_ASSERT(methodName);

	ITypedObject* object = toTypedObject(luaState, 2);
	if (!object) [[unlikely]]
	{
		log::error << L"Unable to call method \"" << mbstows(methodName) << L"\", class " << runtimeClass->getExportType().getName() << L"; null object" << Endl;
		return 0;
	}

	// Convert arguments; first argument should always be method name.
	Any argv[8];
	argv[0] = Any::fromString(methodName);
	ms_instance->toAny(3, top - 2, &argv[1]);

#if T_VERIFY_USING_EXCEPTIONS
	try
#endif
	{
		const Any returnValue = runtimeDispatch->invoke(object, top - 1, argv);
		ms_instance->pushAny(returnValue);
		return 1;
	}
#if T_VERIFY_USING_EXCEPTIONS
	catch(const RuntimeException& x)
	{
		log::error << L"Unhandled RuntimeException occurred when calling unknown method \"" << mbstows(methodName) << L"\", class " << runtimeClass->getExportType().getName() << L"; \"" << x.what() << L"\"." << Endl;
		ms_instance->breakDebugger(luaState);
	}
#endif

	return 0;
}

int ScriptManagerLua::classCallMethod(lua_State* luaState)
{
	const IRuntimeDispatch* runtimeDispatch = reinterpret_cast< const IRuntimeDispatch* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT(runtimeDispatch);

	const int32_t top = lua_gettop(luaState);
	if (top < 1) [[unlikely]]
		return 0;

	ITypedObject* object = toTypedObject(luaState, 1);
	if (!object) [[unlikely]]
	{
		const IRuntimeClass* runtimeClass = reinterpret_cast< const IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(2)));
		log::error << L"Unable to call method \"" << mbstows(findRuntimeClassMethodName(runtimeClass, runtimeDispatch)) << L"\"; null object." << Endl;
		return 0;
	}

	Any argv[10];
	ms_instance->toAny(2, top - 1, argv);

#if T_VERIFY_USING_EXCEPTIONS
	try
#endif
	{
		const Any returnValue = runtimeDispatch->invoke(object, top - 1, argv);
		ms_instance->pushAny(returnValue);
		return 1;
	}
#if T_VERIFY_USING_EXCEPTIONS
	catch(const RuntimeException& x)
	{
		const IRuntimeClass* runtimeClass = reinterpret_cast< const IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(2)));
		log::error << L"Unhandled RuntimeException occurred when calling method \"" << mbstows(findRuntimeClassMethodName(runtimeClass, runtimeDispatch)) << L"\", class " << runtimeClass->getExportType().getName() << L"; \"" << x.what() << L"\"." << Endl;
		ms_instance->breakDebugger(luaState);
	}
#endif

	return 0;
}

int ScriptManagerLua::classCallStaticMethod(lua_State* luaState)
{
	const IRuntimeDispatch* runtimeDispatch = reinterpret_cast< const IRuntimeDispatch* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT(runtimeDispatch);

	const int32_t top = lua_gettop(luaState);
	if (top < 0) [[unlikely]]
		return 0;

	Any argv[10];
	ms_instance->toAny(1, top, argv);

#if T_VERIFY_USING_EXCEPTIONS
	try
#endif
	{
		const Any returnValue = runtimeDispatch->invoke(0, top, argv);
		ms_instance->pushAny(returnValue);
		return 1;
	}
#if T_VERIFY_USING_EXCEPTIONS
	catch(const RuntimeException& x)
	{
		const IRuntimeClass* runtimeClass = reinterpret_cast< const IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(2)));
		log::error << L"Unhandled RuntimeException occurred when calling static method \"" << mbstows(findRuntimeClassMethodName(runtimeClass, runtimeDispatch)) << L"\", class " << runtimeClass->getExportType().getName() << L"; \"" << x.what() << L"\"." << Endl;
		ms_instance->breakDebugger(luaState);
	}
#endif

	return 0;
}

int ScriptManagerLua::classSetProperty(lua_State* luaState)
{
	const IRuntimeDispatch* runtimeDispatch = reinterpret_cast< const IRuntimeDispatch* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT(runtimeDispatch);

	ITypedObject* object = toTypedObject(luaState, 1);
	if (!object) [[unlikely]]
	{
		const IRuntimeClass* runtimeClass = reinterpret_cast< IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(2)));
		log::error << L"Unable to set property \"" << mbstows(findRuntimeClassPropertyName(runtimeClass, runtimeDispatch)) << L"\"; null object" << Endl;
		return 0;
	}

#if T_VERIFY_USING_EXCEPTIONS
	try
#endif
	{
		const Any value = ms_instance->toAny(2);
		runtimeDispatch->invoke(object, 1, &value);
	}
#if T_VERIFY_USING_EXCEPTIONS
	catch(const RuntimeException& x)
	{
		const IRuntimeClass* runtimeClass = reinterpret_cast< IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(2)));
		log::error << L"Unhandled RuntimeException occurred when setting property \"" << mbstows(findRuntimeClassPropertyName(runtimeClass, runtimeDispatch)) << L"\", class " << runtimeClass->getExportType().getName() << L"; \"" << x.what() << L"\"." << Endl;
		ms_instance->breakDebugger(luaState);
	}
#endif

	return 0;
}

int ScriptManagerLua::classGetProperty(lua_State* luaState)
{
	const IRuntimeDispatch* runtimeDispatch = reinterpret_cast< const IRuntimeDispatch* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT(runtimeDispatch);

	ITypedObject* object = toTypedObject(luaState, 1);
	if (!object) [[unlikely]]
	{
		const IRuntimeClass* runtimeClass = reinterpret_cast< IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(2)));
		log::error << L"Unable to get property \"" << mbstows(findRuntimeClassPropertyName(runtimeClass, runtimeDispatch)) << L"\"; null object" << Endl;
		return 0;
	}

	const Any value = runtimeDispatch->invoke(object, 0, 0);
	ms_instance->pushAny(value);
	return 1;
}

int ScriptManagerLua::classEqual(lua_State* luaState)
{
	const Any object0 = ms_instance->toAny(1);
	const Any object1 = ms_instance->toAny(2);

	if (object0.isObject() && object1.isObject())
	{
		if (object0.getObject() == object1.getObject())
			return 1;
	}

	return 0;
}

int ScriptManagerLua::classAdd(lua_State* luaState)
{
	const IRuntimeClass* runtimeClass = reinterpret_cast< const IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT(runtimeClass);

	const IRuntimeDispatch* runtimeDispatch = reinterpret_cast< const IRuntimeDispatch* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT(runtimeDispatch);

	const int32_t top = lua_gettop(luaState);
	if (top < 1) [[unlikely]]
		return 0;

	ITypedObject* object = nullptr;
	Any arg;

	if (lua_istable(luaState, 1))
	{
		object = toTypedObject(luaState, 1);
		arg = ms_instance->toAny(2);
	}
	else if (lua_isuserdata(luaState, 2))
	{
		object = toTypedObject(luaState, 2);
		arg = ms_instance->toAny(1);
	}

	if (!object) [[unlikely]]
	{
		log::error << L"Unable to call add operator, class " << runtimeClass->getExportType().getName() << L"; null object" << Endl;
		return 0;
	}

#if T_VERIFY_USING_EXCEPTIONS
	try
#endif
	{
		const Any returnValue = runtimeDispatch->invoke(object, 1, &arg);
		ms_instance->pushAny(returnValue);
		return 1;
	}
#if T_VERIFY_USING_EXCEPTIONS
	catch(const RuntimeException& x)
	{
		log::error << L"Unhandled RuntimeException occurred when calling add operator, class " << runtimeClass->getExportType().getName() << L"; \"" << x.what() << L"\"." << Endl;
		ms_instance->breakDebugger(luaState);
	}
#endif

	return 0;
}

int ScriptManagerLua::classSubtract(lua_State* luaState)
{
	const IRuntimeClass* runtimeClass = reinterpret_cast< const IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT(runtimeClass);

	const IRuntimeDispatch* runtimeDispatch = reinterpret_cast< const IRuntimeDispatch* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT(runtimeDispatch);

	const int32_t top = lua_gettop(luaState);
	if (top < 1) [[unlikely]]
		return 0;

	ITypedObject* object = toTypedObject(luaState, 1);
	if (!object) [[unlikely]]
	{
		log::error << L"Unable to call subtract operator, class " << runtimeClass->getExportType().getName() << L"; null object" << Endl;
		return 0;
	}

	const Any arg = ms_instance->toAny(2);

#if T_VERIFY_USING_EXCEPTIONS
	try
#endif
	{
		const Any returnValue = runtimeDispatch->invoke(object, 1, &arg);
		ms_instance->pushAny(returnValue);
		return 1;
	}
#if T_VERIFY_USING_EXCEPTIONS
	catch(const RuntimeException& x)
	{
		log::error << L"Unhandled RuntimeException occurred when calling subtract operator, class " << runtimeClass->getExportType().getName() << L"; \"" << x.what() << L"\"." << Endl;
		ms_instance->breakDebugger(luaState);
	}
#endif

	return 0;
}

int ScriptManagerLua::classMultiply(lua_State* luaState)
{
	const IRuntimeClass* runtimeClass = reinterpret_cast< const IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT(runtimeClass);

	const IRuntimeDispatch* runtimeDispatch = reinterpret_cast< const IRuntimeDispatch* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT(runtimeDispatch);

	const int32_t top = lua_gettop(luaState);
	if (top < 1) [[unlikely]]
		return 0;

	ITypedObject* object = nullptr;
	Any arg;

	if (lua_istable(luaState, 1))
	{
		object = toTypedObject(luaState, 1);
		arg = ms_instance->toAny(2);
	}
	else if (lua_isuserdata(luaState, 2))
	{
		object = toTypedObject(luaState, 2);
		arg = ms_instance->toAny(1);
	}

	if (!object) [[unlikely]]
	{
		log::error << L"Unable to call multiply operator, class " << runtimeClass->getExportType().getName() << L"; null object" << Endl;
		return 0;
	}

#if T_VERIFY_USING_EXCEPTIONS
	try
#endif
	{
		const Any returnValue = runtimeDispatch->invoke(object, 1, &arg);
		ms_instance->pushAny(returnValue);
		return 1;
	}
#if T_VERIFY_USING_EXCEPTIONS
	catch(const RuntimeException& x)
	{
		log::error << L"Unhandled RuntimeException occurred when calling multiply operator, class " << runtimeClass->getExportType().getName() << L"; \"" << x.what() << L"\"." << Endl;
		ms_instance->breakDebugger(luaState);
	}
#endif

	return 0;
}

int ScriptManagerLua::classDivide(lua_State* luaState)
{
	const IRuntimeClass* runtimeClass = reinterpret_cast< const IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT(runtimeClass);

	const IRuntimeDispatch* runtimeDispatch = reinterpret_cast< const IRuntimeDispatch* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT(runtimeDispatch);

	const int32_t top = lua_gettop(luaState);
	if (top < 1) [[unlikely]]
		return 0;

	ITypedObject* object = toTypedObject(luaState, 1);
	if (!object) [[unlikely]]
	{
		log::error << L"Unable to call divide operator, class " << runtimeClass->getExportType().getName() << L"; null object" << Endl;
		return 0;
	}

	const Any arg = ms_instance->toAny(2);

#if T_VERIFY_USING_EXCEPTIONS
	try
#endif
	{
		const Any returnValue = runtimeDispatch->invoke(object, 1, &arg);
		ms_instance->pushAny(returnValue);
		return 1;
	}
#if T_VERIFY_USING_EXCEPTIONS
	catch(const RuntimeException& x)
	{
		log::error << L"Unhandled RuntimeException occurred when calling divide operator, class " << runtimeClass->getExportType().getName() << L"; \"" << x.what() << L"\"." << Endl;
		ms_instance->breakDebugger(luaState);
	}
#endif

	return 0;
}

int ScriptManagerLua::classNewIndex(lua_State* luaState)
{
	// lua_upvalueindex(1) == __setters
	// lua_upvalueindex(2) == class

	// 1 [-3] .	table
	// 2 [-2] .	string: "__typename"
	// 3 [-1] .	string: "traktor.Color4f"

	// Check if a property setter.
	DO_0(luaState, lua_pushvalue(luaState, lua_upvalueindex(1))	);
	DO_1(luaState, lua_pushvalue(luaState, -3)					);
	DO_1(luaState, lua_rawget(luaState, -2)						);
	if (lua_isfunction(luaState, -1))
	{
		// Invoke property setter.
		DO_1(luaState, lua_pushvalue(luaState, -5)				);
		DO_1(luaState, lua_pushvalue(luaState, -4)				);
		DO_1(luaState, lua_call(luaState, 2, 1)					);
		return 1;
	}
	DO_1(luaState, lua_pop(luaState, 2)							);

	// Associate value on instance table.
	DO_1(luaState, lua_rawset(luaState, -3)						);
	return 0;
}

int ScriptManagerLua::classIndex(lua_State* luaState)
{
	// lua_upvalueindex(1) == __getters
	// lua_upvalueindex(2) == class

	// Check if a property getter.
	DO_0(luaState, lua_pushvalue(luaState, lua_upvalueindex(1))	);
	DO_1(luaState, lua_pushvalue(luaState, -2)					);
	DO_1(luaState, lua_rawget(luaState, -2)						);
	if (lua_isfunction(luaState, -1))
	{
		// Invoke property getter.
		DO_1(luaState, lua_pushvalue(luaState, -4)				);
		DO_1(luaState, lua_call(luaState, 1, 1)					);
		return 1;
	}
	DO_1(luaState, lua_pop(luaState, 2)							);

	// Check if a method.
	DO_1(luaState, lua_pushvalue(luaState, lua_upvalueindex(2))	);
	DO_1(luaState, lua_pushvalue(luaState, -2)					);
	DO_1(luaState, lua_rawget(luaState, -2)						);

	return 1;
}

void* ScriptManagerLua::luaAlloc(void* ud, void* ptr, size_t osize, size_t nsize)
{
	ScriptManagerLua* this_ = reinterpret_cast< ScriptManagerLua* >(ud);
	T_ASSERT(this_);

	IAllocator* allocator = getAllocator();
	size_t& totalMemoryUse = this_->m_totalMemoryUse;
	if (nsize > 0)
	{
		totalMemoryUse += nsize;

#if defined(T_USE_ALLOCATOR)
		if (osize >= nsize && osize - nsize < 512)
		{
			T_ASSERT(ptr);
			totalMemoryUse -= osize;
			return ptr;
		}

		void* nptr = allocator->alloc(nsize, 16, T_FILE_LINE);
		if (!nptr)
			return nullptr;
#endif

		if (ptr && osize > 0)
		{
#if defined(T_USE_ALLOCATOR)
			std::memcpy(nptr, ptr, std::min(osize, nsize));
			allocator->free(ptr);
#endif
			T_ASSERT(osize <= totalMemoryUse);
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
		T_ASSERT(osize <= totalMemoryUse);
		totalMemoryUse -= osize;
	}

#if defined(T_USE_ALLOCATOR)
	return nullptr;
#else
	return ((lua_Alloc)(this_->m_defaultAllocFn))(this_->m_defaultAllocOpaque, ptr, osize, nsize);
#endif
}

int ScriptManagerLua::luaAllocatedMemory(lua_State* luaState)
{
	lua_pushinteger(luaState, lua_Integer(ms_instance->m_totalMemoryUse));
	return 1;
}

void ScriptManagerLua::hookCallback(lua_State* luaState, lua_Debug* ar)
{
	if (ms_instance->m_debugger)
		ms_instance->m_debugger->hookCallback(luaState, ar);
	if (ms_instance->m_profiler)
		ms_instance->m_profiler->hookCallback(luaState, ar);
}

int ScriptManagerLua::luaPanic(lua_State* luaState)
{
	log::error << L"LUA PANIC; Unrecoverable error \"" << mbstows(lua_tostring(luaState, lua_gettop(luaState))) << L"\"" << Endl;
	return 0;
}

}
