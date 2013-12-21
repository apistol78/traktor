#include <cstring>
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Thread/Acquire.h"
#include "Script/Boxes.h"
#include "Script/IScriptClass.h"
#include "Script/Lua/ScriptContextLua.h"
#include "Script/Lua/ScriptDebuggerLua.h"
#include "Script/Lua/ScriptDelegateLua.h"
#include "Script/Lua/ScriptManagerLua.h"
#include "Script/Lua/ScriptProfilerLua.h"
#include "Script/Lua/ScriptResourceLua.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

const int32_t c_tableKey_class = -1;
const int32_t c_maxTargetSteps = 100;

class TableContainerLua : public Object
{
	T_RTTI_CLASS;

public:
	TableContainerLua(lua_State* luaState)
	:	m_luaState(luaState)
	{
		m_tableRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
	}

	virtual ~TableContainerLua()
	{
		luaL_unref(m_luaState, LUA_REGISTRYINDEX, m_tableRef);
	}

	void push()
	{
		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_tableRef);
		T_ASSERT (lua_istable(m_luaState, -1));
	}

private:
	lua_State* m_luaState;
	int32_t m_tableRef;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.TableContainerLua", TableContainerLua, Object)

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
	m_luaState = luaL_newstate();

	// Hook default allocator to intercept allocation stats.
	m_defaultAllocFn = (void*)lua_getallocf(m_luaState, &m_defaultAllocOpaque);
	T_FATAL_ASSERT (m_defaultAllocFn);
	lua_setallocf(m_luaState, &luaAlloc, this);

	lua_atpanic(m_luaState, luaPanic);

	luaopen_base(m_luaState);
	luaopen_table(m_luaState);
	luaopen_string(m_luaState);
	luaopen_math(m_luaState);
	luaopen_os(m_luaState);
#if defined(LUA_BITLIBNAME)
	luaopen_bit(m_luaState);
#endif

	lua_register(m_luaState, "print", luaPrint);

	registerBoxClasses(this);

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

	m_timer.start();
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

	m_debugger = 0;
	m_profiler = 0;

	while (!m_contexts.empty())
		m_contexts.back()->destroy();

	luaL_unref(m_luaState, LUA_REGISTRYINDEX, m_objectTableRef);
	m_objectTableRef = LUA_NOREF;

	lua_close(m_luaState);
	m_luaState = 0;
}

void ScriptManagerLua::registerClass(IScriptClass* scriptClass)
{
	CHECK_LUA_STACK(m_luaState, 0);

	RegisteredClass rc;

	rc.scriptClass = scriptClass;

	lua_newtable(m_luaState);										// +1	-> 1
	rc.metaTableRef = luaL_ref(m_luaState, LUA_REGISTRYINDEX);		// -1	-> 0
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, rc.metaTableRef);	// +1	-> 1

	lua_pushcfunction(m_luaState, classGcMethod);
	lua_setfield(m_luaState, -2, "__gc");

	lua_pushvalue(m_luaState, -1);									// +1	-> 2
	lua_setfield(m_luaState, -2, "__index");						// -1	-> 1

	lua_pushlightuserdata(m_luaState, (void*)this);
	lua_pushlightuserdata(m_luaState, (void*)scriptClass);
	lua_pushcclosure(m_luaState, classEqualMethod, 2);
	lua_setfield(m_luaState, -2, "__eq");

	lua_pushlightuserdata(m_luaState, (void*)scriptClass);			// +1	-> 2
	lua_rawseti(m_luaState, -2, c_tableKey_class);					// -1	-> 1

	uint32_t staticMethodCount = scriptClass->getStaticMethodCount();
	for (uint32_t i = 0; i < staticMethodCount; ++i)
	{
		std::string methodName = scriptClass->getStaticMethodName(i);
		lua_pushinteger(m_luaState, i);
		lua_pushlightuserdata(m_luaState, (void*)this);
		lua_pushlightuserdata(m_luaState, (void*)scriptClass);
		lua_pushcclosure(m_luaState, classCallStaticMethod, 3);
		lua_setfield(m_luaState, -2, methodName.c_str());
	}

	uint32_t methodCount = scriptClass->getMethodCount();
	for (uint32_t i = 0; i < methodCount; ++i)
	{
		std::string methodName = scriptClass->getMethodName(i);
		lua_pushinteger(m_luaState, i);
		lua_pushlightuserdata(m_luaState, (void*)this);
		lua_pushlightuserdata(m_luaState, (void*)scriptClass);
		lua_pushcclosure(m_luaState, classCallMethod, 3);
		lua_setfield(m_luaState, -2, methodName.c_str());
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

	if (/*exportedAsRoot && */scriptClass->haveUnknown())
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

	// Add class to registry.
	uint32_t classRegistryIndex = m_classRegistry.size();
	m_classRegistry.push_back(rc);

	// Add entries to lookup table; flatten with all specialized types in order
	// to reduce lookups required whe resolving script class.
	TypeInfoSet derivedTypes;
	exportType.findAllOf(derivedTypes);

	for (TypeInfoSet::const_iterator i = derivedTypes.begin(); i != derivedTypes.end(); ++i)
	{
		SmallMap< const TypeInfo*, uint32_t >::const_iterator j = m_classRegistryLookup.find(*i);
		if (j != m_classRegistryLookup.end())
		{
			const RegisteredClass& rc2 = m_classRegistry[j->second];
			const TypeInfo& exportType2 = rc2.scriptClass->getExportType();
			if (is_type_of(exportType, exportType2))
				continue;
		}
		m_classRegistryLookup[*i] = classRegistryIndex;
	}
}

Ref< IScriptResource > ScriptManagerLua::compile(const std::wstring& fileName, const std::wstring& script, const source_map_t* map, IErrorCallback* errorCallback) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
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
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	CHECK_LUA_STACK(m_luaState, 0);

	const ScriptResourceLua* scriptResourceLua = checked_type_cast< const ScriptResourceLua*, false >(scriptResource);

	// Create local environment.
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

	// Create new context.
	Ref< ScriptContextLua > context = new ScriptContextLua(
		this,
		m_luaState,
		environmentRef,
		scriptResourceLua->m_map
	);

	// Call script.
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, environmentRef);
	lua_setfenv(m_luaState, -2);
	lua_call(m_luaState, 0, 0);

	// Copy global member values from prototype.
	if (contextPrototype)
	{
		const ScriptContextLua* sourceContext = checked_type_cast< const ScriptContextLua*, false >(contextPrototype);

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

	m_contexts.push_back(context);
	return context;
}

Ref< IScriptDebugger > ScriptManagerLua::createDebugger()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!m_debugger)
		m_debugger = new ScriptDebuggerLua(this, m_luaState);

	return m_debugger;
}

Ref< IScriptProfiler > ScriptManagerLua::createProfiler()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!m_profiler)
		m_profiler = new ScriptProfilerLua(this, m_luaState);

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

void ScriptManagerLua::lock(ScriptContextLua* context)
{
	m_lock.wait();
	m_lockContext = context;
}

void ScriptManagerLua::unlock()
{
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

	// If this is a wrapped LUA table or function then unwrap and push as is.
	if (&type_of(object) == &type_of< TableContainerLua >())
	{
		TableContainerLua* tableContainer = checked_type_cast< TableContainerLua*, false >(object);
		tableContainer->push();
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

	// Find registered script class entry.
	SmallMap< const TypeInfo*, uint32_t >::const_iterator i = m_classRegistryLookup.find(objectType);
	if (i == m_classRegistryLookup.end())
	{
		lua_pop(m_luaState, 1);
		lua_pushnil(m_luaState);
		return;
	}

	const RegisteredClass& rc = m_classRegistry[i->second];

	Object** objectRef = reinterpret_cast< Object** >(lua_newuserdata(m_luaState, sizeof(Object*)));
	*objectRef = object;
	T_SAFE_ADDREF(*objectRef);

	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, rc.metaTableRef);
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

Any ScriptManagerLua::toAny(int32_t index)
{
	CHECK_LUA_STACK(m_luaState, 0);

	if (lua_isnumber(m_luaState, index))
		return Any::fromFloat(float(lua_tonumber(m_luaState, index)));
	if (lua_isboolean(m_luaState, index))
		return Any::fromBoolean(bool(lua_toboolean(m_luaState, index) != 0));
	if (lua_isstring(m_luaState, index))
		return Any::fromString(lua_tostring(m_luaState, index));
	if (lua_isuserdata(m_luaState, index))
	{
		Object* object = *reinterpret_cast< Object** >(lua_touserdata(m_luaState, index));
		if (object)
			return Any::fromObject(object);
	}
	if (lua_isfunction(m_luaState, index))
	{
		// Box LUA function into C++ container.
		lua_pushvalue(m_luaState, index);
		return Any::fromObject(new ScriptDelegateLua(this, m_lockContext, m_luaState));
	}
	if (lua_istable(m_luaState, index))
	{
		// Unbox wrapped native type.
		lua_rawgeti(m_luaState, index, c_tableKey_class);
		if (lua_isuserdata(m_luaState, -1))
		{
			const IScriptClass* scriptClass = reinterpret_cast< const IScriptClass* >(lua_touserdata(m_luaState, -1));
			lua_pop(m_luaState, 1);
			if (scriptClass)
				return Any::fromTypeInfo(&scriptClass->getExportType());
		}
		lua_pop(m_luaState, 1);

		// Box LUA table into C++ container.
		lua_pushvalue(m_luaState, index);
		return Any::fromObject(new TableContainerLua(m_luaState));
	}

	return Any();
}

void ScriptManagerLua::toAny(int32_t base, int32_t count, Any* outAnys)
{
	CHECK_LUA_STACK(m_luaState, 0);

	for (int32_t i = 0; i < count; ++i)
	{
		int32_t index = base + i;

		if (lua_isnumber(m_luaState, index))
			outAnys[i] = Any::fromFloat(float(lua_tonumber(m_luaState, index)));
		else if (lua_isboolean(m_luaState, index))
			outAnys[i] = Any::fromBoolean(bool(lua_toboolean(m_luaState, index) != 0));
		else if (lua_isstring(m_luaState, index))
			outAnys[i] = Any::fromString(lua_tostring(m_luaState, index));
		else if (lua_isuserdata(m_luaState, index))
		{
			Object* object = *reinterpret_cast< Object** >(lua_touserdata(m_luaState, index));
			if (object)
				outAnys[i] = Any::fromObject(object);
		}
		else if (lua_isfunction(m_luaState, index))
		{
			// Box LUA function into C++ container.
			lua_pushvalue(m_luaState, index);
			outAnys[i] = Any::fromObject(new ScriptDelegateLua(this, m_lockContext, m_luaState));
		}
		else if (lua_istable(m_luaState, index))
		{
			// Unbox wrapped native type.
			lua_rawgeti(m_luaState, index, c_tableKey_class);
			if (lua_isuserdata(m_luaState, -1))
			{
				const IScriptClass* scriptClass = reinterpret_cast< const IScriptClass* >(lua_touserdata(m_luaState, -1));
				lua_pop(m_luaState, 1);
				if (scriptClass)
				{
					outAnys[i] = Any::fromTypeInfo(&scriptClass->getExportType());
					continue;
				}
			}
			lua_pop(m_luaState, 1);

			// Box LUA table into C++ container.
			lua_pushvalue(m_luaState, index);
			outAnys[i] = Any::fromObject(new TableContainerLua(m_luaState));
		}
	}
}

void ScriptManagerLua::collectGarbageFull()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	uint32_t memoryUse = 0;
	do
	{
		memoryUse = m_totalMemoryUse;
		lua_gc(m_luaState, LUA_GCCOLLECT, 0);
	}
	while (memoryUse != m_totalMemoryUse);
	m_lastMemoryUse = m_totalMemoryUse;
}

void ScriptManagerLua::collectGarbagePartial()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (m_collectSteps < 0)
	{
		lua_gc(m_luaState, LUA_GCSTOP, 0);
		m_collectSteps = 0;
	}

	m_collectTargetSteps += float(m_timer.getDeltaTime() * m_collectStepFrequency);

	int32_t targetSteps = int32_t(m_collectTargetSteps);
	while (m_collectSteps < targetSteps)
	{
		lua_gc(m_luaState, LUA_GCSTEP, 0);
		++m_collectSteps;
	}

	if (m_lastMemoryUse <= 0)
		m_lastMemoryUse = m_totalMemoryUse;

	if (m_totalMemoryUse > m_lastMemoryUse)
	{
		size_t garbageProduced = m_totalMemoryUse - m_lastMemoryUse;
		m_collectStepFrequency = max< float >(
			clamp((60.0f * garbageProduced) / (16*1024), 1.0f, 200.0f),
			m_collectStepFrequency
		);
	}
	else
	{
		m_collectStepFrequency = max< float >(1.0f, m_collectStepFrequency - 1.0f);
	}

	m_lastMemoryUse = m_totalMemoryUse;
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
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT (manager);

	const IScriptClass* scriptClass = reinterpret_cast< IScriptClass* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (scriptClass);

	int32_t top = lua_gettop(luaState);
	if (top < 1)
		return 0;

	Any argv[8];
	manager->toAny(2, top - 1, argv);

	IScriptClass::InvokeParam param;
	param.object = 0;

	Any returnValue = Any::fromObject(scriptClass->construct(param, top - 1, argv));
	manager->pushAny(returnValue);

	return 1;
}

int ScriptManagerLua::classCallMethod(lua_State* luaState)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (manager);

	const IScriptClass* scriptClass = reinterpret_cast< IScriptClass* >(lua_touserdata(luaState, lua_upvalueindex(3)));
	T_ASSERT (scriptClass);

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

	IScriptClass::InvokeParam param;
	param.object = object;

	Any returnValue = scriptClass->invoke(param, methodId, top - 1, argv);

	manager->pushAny(returnValue);

	return 1;
}

int ScriptManagerLua::classCallStaticMethod(lua_State* luaState)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (manager);

	const IScriptClass* scriptClass = reinterpret_cast< IScriptClass* >(lua_touserdata(luaState, lua_upvalueindex(3)));
	T_ASSERT (scriptClass);

	int32_t methodId = (int32_t)lua_tonumber(luaState, lua_upvalueindex(1));

	int32_t top = lua_gettop(luaState);
	if (top < 0)
		return 0;

	Any argv[10];
	manager->toAny(1, top, argv);

	Any returnValue = scriptClass->invokeStatic(methodId, top, argv);

	manager->pushAny(returnValue);
	return 1;
}

int ScriptManagerLua::classCallUnknownMethod(lua_State* luaState)
{
	ScriptManagerLua* manager = reinterpret_cast< ScriptManagerLua* >(lua_touserdata(luaState, lua_upvalueindex(2)));
	T_ASSERT (manager);

	const IScriptClass* scriptClass = reinterpret_cast< IScriptClass* >(lua_touserdata(luaState, lua_upvalueindex(3)));
	T_ASSERT (scriptClass);

	const char* methodName = lua_tostring(luaState, lua_upvalueindex(1));
	T_ASSERT (methodName);

	int32_t top = lua_gettop(luaState);
	if (top < 1)
		return 0;

	Object* object = *reinterpret_cast< Object** >(lua_touserdata(luaState, 1));
	if (!object)
		return 0;

	Any argv[8];
	manager->toAny(2, top - 1, argv);

	IScriptClass::InvokeParam param;
	param.object = object;

	Any returnValue = scriptClass->invokeUnknown(param, methodName, top - 1, argv);

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

void* ScriptManagerLua::luaAlloc(void* ud, void* ptr, size_t osize, size_t nsize)
{
	ScriptManagerLua* this_ = reinterpret_cast< ScriptManagerLua* >(ud);
	T_ASSERT (this_);

	if (nsize > 0)
	{
		//if (osize >= nsize)
		//	return ptr;

		//void* nptr = getAllocator()->alloc(nsize, 16, "LUA");
		//if (!nptr)
		//	return 0;

		this_->m_totalMemoryUse += nsize;

		if (ptr && osize > 0)
		{
			//std::memcpy(nptr, ptr, std::min(osize, nsize));
			//getAllocator()->free(ptr);

			T_ASSERT (osize <= this_->m_totalMemoryUse);
			this_->m_totalMemoryUse -= osize;
		}

		//return nptr;
	}
	else if (ptr && osize > 0)
	{
		//getAllocator()->free(ptr);

		T_ASSERT (osize <= this_->m_totalMemoryUse);
		this_->m_totalMemoryUse -= osize;
	}
	//return 0;

	return ((lua_Alloc)(this_->m_defaultAllocFn))(this_->m_defaultAllocOpaque, ptr, osize, nsize);
}

int ScriptManagerLua::luaPanic(lua_State* luaState)
{
	log::error << L"LUA PANIC; Unrecoverable error \"" << mbstows(lua_tostring(luaState, lua_gettop(luaState))) << L"\"" << Endl;
	return 0;
}

	}
}
