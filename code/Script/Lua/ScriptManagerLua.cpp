#include <cstring>
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClass.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Save.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
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
#include "Script/Lua/ScriptResourceLua.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

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

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptManagerLua", 0, ScriptManagerLua, IScriptManager)

ScriptManagerLua* ScriptManagerLua::ms_instance = 0;

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
,	m_classIdBoxedVector4(0)
,	m_classIdBoxedTransform(0)
{
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
	ms_instance = 0;
}

void ScriptManagerLua::destroy()
{
	if (!m_luaState)
		return;

	T_ANONYMOUS_VAR(Ref< ScriptManagerLua >)(this);

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
	CHECK_LUA_STACK(m_luaState, 0);

	const TypeInfo& exportType = runtimeClass->getExportType();

	RegisteredClass rc;
	rc.runtimeClass = runtimeClass;

	// Create "class" meta table.
	lua_newtable(m_luaState);
#if defined(_DEBUG)
	lua_pushstring(m_luaState, "class meta");
	lua_setfield(m_luaState, -2, "__name");
#endif

	lua_pushlightuserdata(m_luaState, (void*)runtimeClass);			// +1	-> 2
	lua_rawseti(m_luaState, -2, c_tableKey_class);					// -1	-> 1

	if (runtimeClass->haveUnknown())
	{
		lua_pushlightuserdata(m_luaState, (void*)this);
		lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
		lua_pushcclosure(m_luaState, classIndexLookup, 2);
		lua_setfield(m_luaState, -2, "__index");
	}

	if (runtimeClass->haveConstructor())
	{
		lua_pushlightuserdata(m_luaState, (void*)this);
		lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
		lua_pushcclosure(m_luaState, classCallConstructor, 2);
		lua_setfield(m_luaState, -2, "__call");
	}

	rc.classMetaTableRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);

	// Create class table.
	lua_newtable(m_luaState);										// +1	-> 1
#if defined(_DEBUG)
	lua_pushstring(m_luaState, "class");
	lua_setfield(m_luaState, -2, "__name");
#endif

	lua_pushlightuserdata(m_luaState, (void*)runtimeClass);			// +1	-> 2
	lua_rawseti(m_luaState, -2, c_tableKey_class);					// -1	-> 1

	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, rc.classMetaTableRef);
	lua_setmetatable(m_luaState, -2);

	// Add methods from base classes first.
	std::vector< const TypeInfo* > superTypes;
	for (const TypeInfo* superType = exportType.getSuper(); superType; superType = superType->getSuper())
		superTypes.push_back(superType);

	for (std::vector< const TypeInfo* >::reverse_iterator i = superTypes.rbegin(); i != superTypes.rend(); ++i)
	{
		for (std::vector< RegisteredClass >::iterator j = m_classRegistry.begin(); j != m_classRegistry.end(); ++j)
		{
			if (*i == &j->runtimeClass->getExportType())
			{
				uint32_t staticMethodCount = j->runtimeClass->getStaticMethodCount();
				for (uint32_t i = 0; i < staticMethodCount; ++i)
				{
					std::string methodName = j->runtimeClass->getStaticMethodName(i);
					lua_pushinteger(m_luaState, i);
					lua_pushlightuserdata(m_luaState, (void*)this);
					lua_pushlightuserdata(m_luaState, (void*)j->runtimeClass);
					lua_pushcclosure(m_luaState, classCallStaticMethod, 3);
					lua_setfield(m_luaState, -2, methodName.c_str());
				}

				uint32_t methodCount = j->runtimeClass->getMethodCount();
				for (uint32_t i = 0; i < methodCount; ++i)
				{
					std::string methodName = j->runtimeClass->getMethodName(i);
					lua_pushinteger(m_luaState, i);
					lua_pushlightuserdata(m_luaState, (void*)this);
					lua_pushlightuserdata(m_luaState, (void*)j->runtimeClass);
					lua_pushcclosure(m_luaState, classCallMethod, 3);
					lua_setfield(m_luaState, -2, methodName.c_str());
				}

				break;
			}
		}
	}

	// Add this class's methods.
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

	rc.classTableRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);

	// Create "instance" meta table.
	lua_newtable(m_luaState);
#if defined(_DEBUG)
	lua_pushstring(m_luaState, "instance meta");
	lua_setfield(m_luaState, -2, "__name");
#endif

	lua_pushlightuserdata(m_luaState, (void*)runtimeClass);			// +1	-> 2
	lua_rawseti(m_luaState, -2, c_tableKey_class);					// -1	-> 1

	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, rc.classTableRef);
	lua_setfield(m_luaState, -2, "__index");						// -1	-> 1

	lua_pushcfunction(m_luaState, classGcMethod);					// +1	-> 2
	lua_setfield(m_luaState, -2, "__gc");							// -1	-> 1

	lua_pushlightuserdata(m_luaState, (void*)this);
	lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
	lua_pushcclosure(m_luaState, classEqualMethod, 2);
	lua_setfield(m_luaState, -2, "__eq");

	lua_pushlightuserdata(m_luaState, (void*)this);
	lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
	lua_pushcclosure(m_luaState, classAddMethod, 2);
	lua_setfield(m_luaState, -2, "__add");

	lua_pushlightuserdata(m_luaState, (void*)this);
	lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
	lua_pushcclosure(m_luaState, classSubtractMethod, 2);
	lua_setfield(m_luaState, -2, "__sub");

	lua_pushlightuserdata(m_luaState, (void*)this);
	lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
	lua_pushcclosure(m_luaState, classMultiplyMethod, 2);
	lua_setfield(m_luaState, -2, "__mul");

	lua_pushlightuserdata(m_luaState, (void*)this);
	lua_pushlightuserdata(m_luaState, (void*)runtimeClass);
	lua_pushcclosure(m_luaState, classDivideMethod, 2);
	lua_setfield(m_luaState, -2, "__div");

	rc.instanceMetaTableRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);

	//// Export class in global scope.
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

	// Add class to registry.
	uint32_t classRegistryIndex = m_classRegistry.size();
	m_classRegistry.push_back(rc);

	// Add entries to lookup table; flatten with all specialized types in order
	// to reduce lookups required when resolving script class.
	TypeInfoSet derivedTypes;
	exportType.findAllOf(derivedTypes);

	for (TypeInfoSet::const_iterator i = derivedTypes.begin(); i != derivedTypes.end(); ++i)
	{
		SmallMap< const TypeInfo*, uint32_t >::const_iterator j = m_classRegistryLookup.find(*i);
		if (j != m_classRegistryLookup.end())
		{
			const RegisteredClass& rc2 = m_classRegistry[j->second];
			const TypeInfo& exportType2 = rc2.runtimeClass->getExportType();
			if (is_type_of(exportType, exportType2))
				continue;
		}
		m_classRegistryLookup[*i] = classRegistryIndex;
	}

	// Remember commonly used classes.
	if (is_type_a< BoxedVector4 >(exportType))
		m_classIdBoxedVector4 = classRegistryIndex;
	else if (is_type_a< BoxedTransform >(exportType))
		m_classIdBoxedTransform = classRegistryIndex;

	// Add constants last as constants might be instances of this class.
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, rc.classTableRef);
	for (uint32_t i = 0; i < runtimeClass->getConstantCount(); ++i)
	{
		pushAny(runtimeClass->getConstantValue(i));
		lua_setfield(m_luaState, -2, runtimeClass->getConstantName(i).c_str());
	}
	lua_pop(m_luaState, 1);
}

Ref< IScriptResource > ScriptManagerLua::compile(const std::wstring& fileName, const std::wstring& script, const source_map_t* map, IErrorCallback* errorCallback) const
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

		if (map)
		{
			for (source_map_t::const_reverse_iterator i = map->rbegin(); i != map->rend(); ++i)
			{
				if (line >= i->line)
				{
					if (errorCallback)
						errorCallback->syntaxError(i->name, line - i->line, error);
					else
						log::error << i->name << L" (" << (line - i->line) << L"): " << error << Endl;
					break;
				}
			}
		}
		else
		{
			if (errorCallback)
				errorCallback->syntaxError(fileName, line, error);
			else
				log::error << fileName << L" (" << line << L"): " << error << Endl;
		}

		lua_pop(m_luaState, 1);
		return 0;
	}

	lua_pop(m_luaState, 1);

	Ref< ScriptResourceLua > resource = new ScriptResourceLua();
	resource->m_fileName = wstombs(fileName);
	resource->m_map = map ? *map : source_map_t();
	resource->m_script = text;

	return resource;
}

Ref< IScriptContext > ScriptManagerLua::createContext(const IScriptResource* scriptResource, const IScriptContext* contextPrototype)
{
#if defined(T_SCRIPT_LUA_USE_MT_LOCK)
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
#endif
	CHECK_LUA_STACK(m_luaState, 0);

	const ScriptResourceLua* scriptResourceLua = checked_type_cast< const ScriptResourceLua* >(scriptResource);

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

	// Load script into environment.
	Ref< ScriptContextLua > context;
	if (scriptResourceLua)
	{
		std::string fileName = "@" + scriptResourceLua->m_fileName;
		int32_t result;

		result = luaL_loadbuffer(
			m_luaState,
			(const char*)scriptResourceLua->m_script.c_str(),
			scriptResourceLua->m_script.length(),
			fileName.c_str()
		);

		if (result != 0)
		{
			log::error << L"LUA load error \"" << mbstows(lua_tostring(m_luaState, -1)) << L"\"" << Endl;
			lua_pop(m_luaState, 1);
			return 0;
		}

		context = new ScriptContextLua(
			this,
			m_luaState,
			environmentRef,
			scriptResourceLua->m_map
		);
	}
	else
	{
		context = new ScriptContextLua(
			this,
			m_luaState,
			environmentRef,
			source_map_t()
		);
	}

	// Copy values from prototype; First attempt to have globals ready when calling script.
	if (contextPrototype)
	{
		const ScriptContextLua* sourceContext = checked_type_cast< const ScriptContextLua*, false >(contextPrototype);

		// Copy globals.
		int32_t top = lua_gettop(m_luaState) + 1;

		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, sourceContext->m_environmentRef);
		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, environmentRef);
		lua_pushnil(m_luaState);

		// -3 = sourceContext->m_environmentRef
		// -2 = environmentRef
		// -1 = nil
		while (lua_next(m_luaState, -3))
		{
			// -4 = sourceContext->m_environmentRef
			// -3 = environmentRef
			// -2 = key
			// -1 = value
			if (lua_isfunction(m_luaState, -1))
			{
				lua_pop(m_luaState, 1);

				// -3 = sourceContext->m_environmentRef
				// -2 = environmentRef
				// -1 = key
				continue;
			}

			// -4 = sourceContext->m_environmentRef
			// -3 = environmentRef
			// -2 = key
			// -1 = value
			lua_pop(m_luaState, 1);

			// -3 = sourceContext->m_environmentRef
			// -2 = environmentRef
			// -1 = key
			lua_pushvalue(m_luaState, -1);
			lua_pushvalue(m_luaState, -1);

			// -5 = sourceContext->m_environmentRef
			// -4 = environmentRef
			// -3 = key
			// -2 = key
			// -1 = key
			lua_rawget(m_luaState, -5);

			// -5 = sourceContext->m_environmentRef
			// -4 = environmentRef
			// -3 = key
			// -2 = key
			// -1 = prototype value
			lua_rawset(m_luaState, -4);

			// -3 = sourceContext->m_environmentRef
			// -2 = environmentRef
			// -1 = key
		}

		// -2 = sourceContext->m_environmentRef
		// -1 = environmentRef
		lua_pop(m_luaState, 2);
	}

	// Call script.
	if (scriptResourceLua)
	{
		T_ANONYMOUS_VAR(Save< ScriptContextLua* >)(m_lockContext, context);

		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, environmentRef);
#if defined(T_LUA_5_2)
		lua_setupvalue(m_luaState, -2, 1);
		lua_call(m_luaState, 0, 0);
#else
		lua_setfenv(m_luaState, -2);
		lua_call(m_luaState, 0, 0);
#endif

		// Copy values from prototype again in case they we're overwritten
		// when calling script.
		if (contextPrototype)
		{
			const ScriptContextLua* sourceContext = checked_type_cast< const ScriptContextLua*, false >(contextPrototype);

			// Copy globals.
			int32_t top = lua_gettop(m_luaState) + 1;

			lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, sourceContext->m_environmentRef);
			lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, environmentRef);
			lua_pushnil(m_luaState);

			// -3 = sourceContext->m_environmentRef
			// -2 = environmentRef
			// -1 = nil
			while (lua_next(m_luaState, -3))
			{
				// -4 = sourceContext->m_environmentRef
				// -3 = environmentRef
				// -2 = key
				// -1 = value
				if (lua_isfunction(m_luaState, -1))
				{
					lua_pop(m_luaState, 1);

					// -3 = sourceContext->m_environmentRef
					// -2 = environmentRef
					// -1 = key
					continue;
				}

				// -4 = sourceContext->m_environmentRef
				// -3 = environmentRef
				// -2 = key
				// -1 = value
				lua_pop(m_luaState, 1);

				// -3 = sourceContext->m_environmentRef
				// -2 = environmentRef
				// -1 = key
				lua_pushvalue(m_luaState, -1);
				lua_pushvalue(m_luaState, -1);

				// -5 = sourceContext->m_environmentRef
				// -4 = environmentRef
				// -3 = key
				// -2 = key
				// -1 = key
				lua_rawget(m_luaState, -5);

				// -5 = sourceContext->m_environmentRef
				// -4 = environmentRef
				// -3 = key
				// -2 = key
				// -1 = prototype value
				lua_rawset(m_luaState, -4);

				// -3 = sourceContext->m_environmentRef
				// -2 = environmentRef
				// -1 = key
			}

			// -2 = sourceContext->m_environmentRef
			// -1 = environmentRef
			lua_pop(m_luaState, 2);
		}
	}

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
	outStatistics.memoryUsage = m_totalMemoryUse;
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
	if (&type_of(object) == &type_of< ScriptObjectLua >())
	{
		ScriptObjectLua* scriptObject = checked_type_cast< ScriptObjectLua*, false >(object);
		scriptObject->push();
		return;
	}
	else if (&type_of(object) == &type_of< ScriptDelegateLua >())
	{
		ScriptDelegateLua* delegateContainer = checked_type_cast< ScriptDelegateLua*, false >(object);
		delegateContainer->push();
		return;
	}

	// Have we already pushed this object before and it's still alive in script-land then reuse it.
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_objectTableRef);
	lua_rawgeti(m_luaState, -1, int32_t(uint64_t(object)));
	if (lua_isuserdata(m_luaState, -1))
	{
		lua_remove(m_luaState, -2);
		return;
	}

	// No reference; drop nil value from stack.
	lua_pop(m_luaState, 1);

	const TypeInfo* objectType = &type_of(object);
	uint32_t classId = 0;

	// Check commonly used boxes.
	if (objectType == &type_of< BoxedVector4 >())
	{
		classId = m_classIdBoxedVector4;
	}
	else if (objectType == &type_of< BoxedTransform >())
	{
		classId = m_classIdBoxedTransform;
	}
	else
	{
		// Find registered script class entry.
		SmallMap< const TypeInfo*, uint32_t >::const_iterator i = m_classRegistryLookup.find(objectType);
		if (i != m_classRegistryLookup.end())
			classId = i->second;
		else
		{
			lua_pop(m_luaState, 1);
			lua_pushnil(m_luaState);
			return;
		}
	}

	const RegisteredClass& rc = m_classRegistry[classId];

	ITypedObject** objectRef = reinterpret_cast< ITypedObject** >(lua_newuserdata(m_luaState, sizeof(ITypedObject*)));
	*objectRef = object;
	T_SAFE_ADDREF(*objectRef);

	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, rc.instanceMetaTableRef);
	lua_setmetatable(m_luaState, -2);

	// Store object in weak table.
	lua_rawseti(m_luaState, -2, int32_t(uint64_t(object)));
	lua_rawgeti(m_luaState, -1, int32_t(uint64_t(object)));

	// Remove weak table from stack.
	lua_remove(m_luaState, -2);
}

void ScriptManagerLua::pushAny(const Any& any)
{
	CHECK_LUA_STACK(m_luaState, 1);

	if (any.isVoid())
		lua_pushnil(m_luaState);
	else if (any.isBoolean())
		lua_pushboolean(m_luaState, any.getBooleanUnsafe() ? 1 : 0);
	else if (any.isInteger())
		lua_pushinteger(m_luaState, any.getIntegerUnsafe());
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
		if (any.isVoid())
			lua_pushnil(m_luaState);
		else if (any.isBoolean())
			lua_pushboolean(m_luaState, any.getBooleanUnsafe() ? 1 : 0);
		else if (any.isInteger())
			lua_pushinteger(m_luaState, any.getIntegerUnsafe());
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
		return Any::fromFloat(float(lua_tonumber(m_luaState, index)));
	else if (type == LUA_TBOOLEAN)
		return Any::fromBoolean(bool(lua_toboolean(m_luaState, index) != 0));
	else if (type == LUA_TSTRING)
		return Any::fromString(lua_tostring(m_luaState, index));
	else if (type == LUA_TUSERDATA)
	{
		Object* object = *reinterpret_cast< Object** >(lua_touserdata(m_luaState, index));
		if (object)
			return Any::fromObject(object);
	}
	else if (type == LUA_TFUNCTION)
	{
		// Box LUA function into C++ container.
		lua_pushvalue(m_luaState, index);
		return Any::fromObject(new ScriptDelegateLua(m_lockContext, m_luaState));
	}
	else if (type == LUA_TTABLE)
	{
		// Unbox wrapped native type.
		lua_rawgeti(m_luaState, index, c_tableKey_class);
		if (lua_isuserdata(m_luaState, -1))
		{
			const IRuntimeClass* runtimeClass = reinterpret_cast< const IRuntimeClass* >(lua_touserdata(m_luaState, -1));
			lua_pop(m_luaState, 1);
			if (runtimeClass)
				return Any::fromTypeInfo(&runtimeClass->getExportType());
		}
		lua_pop(m_luaState, 1);

		// Box LUA object into C++ container.
		lua_pushvalue(m_luaState, index);
		int32_t tableRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
		return Any::fromObject(new ScriptObjectLua(m_luaState, tableRef, 0));
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
			outAnys[i] = Any::fromFloat(float(lua_tonumber(m_luaState, index)));
		else if (type == LUA_TBOOLEAN)
			outAnys[i] = Any::fromBoolean(bool(lua_toboolean(m_luaState, index) != 0));
		else if (type == LUA_TSTRING)
			outAnys[i] = Any::fromString(lua_tostring(m_luaState, index));
		else if (type == LUA_TUSERDATA)
		{
			Object* object = *reinterpret_cast< Object** >(lua_touserdata(m_luaState, index));
			if (object)
				outAnys[i] = Any::fromObject(object);
		}
		else if (type == LUA_TFUNCTION)
		{
			// Box LUA function into C++ container.
			lua_pushvalue(m_luaState, index);
			outAnys[i] = Any::fromObject(new ScriptDelegateLua(m_lockContext, m_luaState));
		}
		else if (type == LUA_TTABLE)
		{
			// Unbox wrapped native type.
			lua_rawgeti(m_luaState, index, c_tableKey_class);
			if (lua_isuserdata(m_luaState, -1))
			{
				const IRuntimeClass* runtimeClass = reinterpret_cast< const IRuntimeClass* >(lua_touserdata(m_luaState, -1));
				lua_pop(m_luaState, 1);
				if (runtimeClass)
				{
					outAnys[i] = Any::fromTypeInfo(&runtimeClass->getExportType());
					continue;
				}
			}
			lua_pop(m_luaState, 1);

			// Box LUA object into C++ container.
			lua_pushvalue(m_luaState, index);
			int32_t tableRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
			outAnys[i] = Any::fromObject(new ScriptObjectLua(m_luaState, tableRef, 0));
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
#if defined(T_LUA_5_2)

	lua_gc(m_luaState, LUA_GCCOLLECT, 0);

#else
	uint32_t memoryUse = 0;
	do
	{
		memoryUse = m_totalMemoryUse;
		lua_gc(m_luaState, LUA_GCCOLLECT, 0);
	}
	while (memoryUse != m_totalMemoryUse);
	m_lastMemoryUse = m_totalMemoryUse;
#endif
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

	lua_Debug ar;

	std::memset(&ar, 0, sizeof(ar));
	lua_getstack(luaState, 1, &ar);
	lua_getinfo(luaState, "Snlu", &ar);

	m_debugger->actionBreak();
	m_debugger->analyzeState(luaState, &ar);
}

int ScriptManagerLua::classIndexLookup(lua_State* luaState)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT (manager);

	// Get script class from closure.
	const IRuntimeClass* runtimeClass = reinterpret_cast< IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	if (!runtimeClass)
		return 0;

	// Get index key.
	const char* key = lua_tostring(luaState, 2);
	T_ASSERT (key);

	// Not a member, create "unknown method" closure.
	lua_pushstring(luaState, key);
	lua_pushlightuserdata(luaState, (void*)manager);
	lua_pushlightuserdata(luaState, (void*)runtimeClass);
	lua_pushcclosure(luaState, classCallUnknownMethod, 3);

	return 1;
}

int ScriptManagerLua::classCallConstructor(lua_State* luaState)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT (manager);

	const IRuntimeClass* runtimeClass = reinterpret_cast< IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (runtimeClass);

	int32_t top = lua_gettop(luaState);
	if (top < 1)
		return 0;

	Any argv[8];
	manager->toAny(2, top - 1, argv);

	Any returnValue = Any::fromObject(runtimeClass->construct(top - 1, argv));
	manager->pushAny(returnValue);

	return 1;
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

	Object** objectPtr = reinterpret_cast< Object** >(lua_touserdata(luaState, 1));
	if (!objectPtr)
	{
		log::error << L"Unable to call method; not an object" << Endl;
		return 0;
	}

	Object* object = *objectPtr;
	if (!object)
	{
		log::error << L"Unable to call method; null object" << Endl;
		return 0;
	}

	Any argv[10];
	manager->toAny(2, top - 1, argv);

	Any returnValue = runtimeClass->invoke(object, methodId, top - 1, argv);
	manager->pushAny(returnValue);

	return 1;
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

	Any returnValue = runtimeClass->invokeStatic(methodId, top, argv);
	manager->pushAny(returnValue);

	return 1;
}

int ScriptManagerLua::classCallUnknownMethod(lua_State* luaState)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (manager);

	const IRuntimeClass* runtimeClass = reinterpret_cast< IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(3)));
	T_ASSERT (runtimeClass);

	const char* methodName = lua_tostring(luaState, lua_upvalueindex(1));
	T_ASSERT (methodName);

	int32_t top = lua_gettop(luaState);
	if (top < 1)
		return 0;

	ITypedObject* object = *reinterpret_cast< ITypedObject** >(lua_touserdata(luaState, 1));
	if (!object)
		return 0;

	Any argv[8];
	manager->toAny(2, top - 1, argv);

	Any returnValue = runtimeClass->invokeUnknown(object, methodName, top - 1, argv);
	manager->pushAny(returnValue);

	return 1;
}

int ScriptManagerLua::classGcMethod(lua_State* luaState)
{
	Object** object = reinterpret_cast< Object** >(lua_touserdata(luaState, 1));
	if (object)
	{
		T_SAFE_ANONYMOUS_RELEASE(*object);
	}
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

int ScriptManagerLua::classAddMethod(lua_State* luaState)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT (manager);

	const IRuntimeClass* runtimeClass = reinterpret_cast< IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (runtimeClass);

	int32_t top = lua_gettop(luaState);
	if (top < 1)
		return 0;

	Object** objectPtr = 0;
	Any arg;

	if (lua_isuserdata(luaState, 1))
	{
		objectPtr = reinterpret_cast< Object** >(lua_touserdata(luaState, 1));
		arg = manager->toAny(2);
	}
	else if (lua_isuserdata(luaState, 2))
	{
		objectPtr = reinterpret_cast< Object** >(lua_touserdata(luaState, 2));
		arg = manager->toAny(1);
	}

	if (!objectPtr)
	{
		log::error << L"Unable to call method; not an object" << Endl;
		return 0;
	}

	Object* object = *objectPtr;
	if (!object)
	{
		log::error << L"Unable to call method; null object" << Endl;
		return 0;
	}

	Any returnValue = runtimeClass->invokeOperator(object, 0, arg);
	manager->pushAny(returnValue);

	return 1;
}

int ScriptManagerLua::classSubtractMethod(lua_State* luaState)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT (manager);

	const IRuntimeClass* runtimeClass = reinterpret_cast< IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (runtimeClass);

	int32_t top = lua_gettop(luaState);
	if (top < 1)
		return 0;

	Object** objectPtr = reinterpret_cast< Object** >(lua_touserdata(luaState, 1));
	if (!objectPtr)
	{
		log::error << L"Unable to call method; not an object" << Endl;
		return 0;
	}

	Object* object = *objectPtr;
	if (!object)
	{
		log::error << L"Unable to call method; null object" << Endl;
		return 0;
	}

	Any arg = manager->toAny(2);

	Any returnValue = runtimeClass->invokeOperator(object, 1, arg);
	manager->pushAny(returnValue);

	return 1;
}

int ScriptManagerLua::classMultiplyMethod(lua_State* luaState)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT (manager);

	const IRuntimeClass* runtimeClass = reinterpret_cast< IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (runtimeClass);

	int32_t top = lua_gettop(luaState);
	if (top < 1)
		return 0;

	Object** objectPtr = 0;
	Any arg;

	if (lua_isuserdata(luaState, 1))
	{
		objectPtr = reinterpret_cast< Object** >(lua_touserdata(luaState, 1));
		arg = manager->toAny(2);
	}
	else if (lua_isuserdata(luaState, 2))
	{
		objectPtr = reinterpret_cast< Object** >(lua_touserdata(luaState, 2));
		arg = manager->toAny(1);
	}

	if (!objectPtr)
	{
		log::error << L"Unable to call method; not an object" << Endl;
		return 0;
	}

	Object* object = *objectPtr;
	if (!object)
	{
		log::error << L"Unable to call method; null object" << Endl;
		return 0;
	}

	Any returnValue = runtimeClass->invokeOperator(object, 2, arg);
	manager->pushAny(returnValue);

	return 1;
}

int ScriptManagerLua::classDivideMethod(lua_State* luaState)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT (manager);

	const IRuntimeClass* runtimeClass = reinterpret_cast< IRuntimeClass* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (runtimeClass);

	int32_t top = lua_gettop(luaState);
	if (top < 1)
		return 0;

	Object** objectPtr = reinterpret_cast< Object** >(lua_touserdata(luaState, 1));
	if (!objectPtr)
	{
		log::error << L"Unable to call method; not an object" << Endl;
		return 0;
	}

	Object* object = *objectPtr;
	if (!object)
	{
		log::error << L"Unable to call method; null object" << Endl;
		return 0;
	}

	Any arg = manager->toAny(2);

	Any returnValue = runtimeClass->invokeOperator(object, 3, arg);
	manager->pushAny(returnValue);

	return 1;
}

void* ScriptManagerLua::luaAlloc(void* ud, void* ptr, size_t osize, size_t nsize)
{
	ScriptManagerLua* this_ = reinterpret_cast< ScriptManagerLua* >(ud);
	T_ASSERT (this_);

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

		void* nptr = getAllocator()->alloc(nsize, 16, T_FILE_LINE);
		if (!nptr)
			return 0;
#endif

		if (ptr && osize > 0)
		{
#if defined(T_USE_ALLOCATOR)
			std::memcpy(nptr, ptr, std::min(osize, nsize));
			getAllocator()->free(ptr);
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
		getAllocator()->free(ptr);
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
}
