#ifndef traktor_script_ScriptManagerLua_H
#define traktor_script_ScriptManagerLua_H

#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Timer/Timer.h"
#include "Script/IScriptManager.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_LUA_EXPORT) || defined(T_SCRIPT_LUAJIT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

struct lua_Debug;
struct lua_State;

namespace traktor
{
	namespace script
	{

class Any;
class ScriptContextLua;
class ScriptDebuggerLua;
class ScriptProfilerLua;

/*! \brief LUA script manager.
 * \ingroup LUA Script
 */
class T_DLLCLASS ScriptManagerLua : public IScriptManager
{
	T_RTTI_CLASS;

public:
	ScriptManagerLua();

	virtual ~ScriptManagerLua();

	virtual void destroy();
	
	virtual void registerClass(IScriptClass* scriptClass);

	virtual Ref< IScriptResource > compile(const std::wstring& fileName, const std::wstring& script, const source_map_t* map, IErrorCallback* errorCallback) const;

	virtual Ref< IScriptContext > createContext(const IScriptResource* scriptResource, const IScriptContext* contextPrototype);

	virtual Ref< IScriptDebugger > createDebugger();

	virtual Ref< IScriptProfiler > createProfiler();

	virtual void collectGarbage(bool full);

	virtual void getStatistics(ScriptStatistics& outStatistics) const;

private:
	friend class ScriptContextLua;
	friend class ScriptDebuggerLua;
	friend class ScriptDelegateLua;
	friend class ScriptProfilerLua;

	struct RegisteredClass
	{
		Ref< IScriptClass > scriptClass;
		int32_t metaTableRef;
	};

	static ScriptManagerLua* ms_instance;

	lua_State* m_luaState;
	void* m_defaultAllocFn;
	void* m_defaultAllocOpaque;
	int32_t m_objectTableRef;
	mutable Semaphore m_lock;
	std::vector< RegisteredClass > m_classRegistry;
	SmallMap< const TypeInfo*, uint32_t > m_classRegistryLookup;
	RefArray< ScriptContextLua > m_contexts;
	ScriptContextLua* m_lockContext;
	Ref< ScriptDebuggerLua > m_debugger;
	Ref< ScriptProfilerLua > m_profiler;
	Timer m_timer;
	float m_collectStepFrequency;
	int32_t m_collectSteps;
	float m_collectTargetSteps;
	size_t m_totalMemoryUse;
	size_t m_lastMemoryUse;
	uint32_t m_classIdBoxedVector4;
	uint32_t m_classIdBoxedTransform;

	void destroyContext(ScriptContextLua* context);

	void lock(ScriptContextLua* context);

	void unlock();

	void pushObject(ITypedObject* object);

	void pushAny(const Any& any);

	Any toAny(int32_t index);

	void toAny(int32_t base, int32_t count, Any* outAnys);

	void collectGarbageFull();

	void collectGarbagePartial();

	void breakDebugger(lua_State* luaState);

	static int classIndexLookup(lua_State* luaState);

	static int classCallConstructor(lua_State* luaState);

	static int classCallMethod(lua_State* luaState);

	static int classCallStaticMethod(lua_State* luaState);

	static int classCallUnknownMethod(lua_State* luaState);

	static int classGcMethod(lua_State* luaState);

	static int classEqualMethod(lua_State* luaState);

	static int classAddMethod(lua_State* luaState);
	
	static int classSubtractMethod(lua_State* luaState);
	
	static int classMultiplyMethod(lua_State* luaState);

	static int classDivideMethod(lua_State* luaState);

	static void* luaAlloc(void* ud, void* ptr, size_t osize, size_t nsize);

	static void hookCallback(lua_State* L, lua_Debug* ar);

	static int luaPanic(lua_State* luaState);
};

	}
}

#endif	// traktor_script_ScriptManagerLua_H
