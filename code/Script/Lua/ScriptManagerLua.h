#ifndef traktor_script_ScriptManagerLua_H
#define traktor_script_ScriptManagerLua_H

#define T_SCRIPT_LUA_USE_MT_LOCK

#include "Core/RefArray.h"
#include "Script/IScriptManager.h"

#if defined(T_SCRIPT_LUA_USE_MT_LOCK)
#	include "Core/Thread/Semaphore.h"
#endif

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

class Any;

	namespace script
	{

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

	virtual void destroy() T_OVERRIDE T_FINAL;
	
	virtual void registerClass(IRuntimeClass* runtimeClass) T_OVERRIDE T_FINAL;

	virtual Ref< IScriptBlob > compile(const std::wstring& fileName, const std::wstring& script, IErrorCallback* errorCallback) const T_OVERRIDE T_FINAL;

	virtual Ref< IScriptContext > createContext(bool strict) T_OVERRIDE T_FINAL;

	virtual Ref< IScriptDebugger > createDebugger() T_OVERRIDE T_FINAL;

	virtual Ref< IScriptProfiler > createProfiler() T_OVERRIDE T_FINAL;

	virtual void collectGarbage(bool full) T_OVERRIDE T_FINAL;

	virtual void getStatistics(ScriptStatistics& outStatistics) const T_OVERRIDE T_FINAL;

private:
	friend class ScriptClassLua;
	friend class ScriptContextLua;
	friend class ScriptDebuggerLua;
	friend class ScriptDelegateLua;
	friend class ScriptProfilerLua;

	struct RegisteredClass
	{
		Ref< IRuntimeClass > runtimeClass;
		int32_t classTableRef;
		int32_t classMetaTableRef;
		int32_t instanceMetaTableRef;
	};

	static ScriptManagerLua* ms_instance;

	lua_State* m_luaState;
	void* m_defaultAllocFn;
	void* m_defaultAllocOpaque;
	int32_t m_objectTableRef;
#if defined(T_SCRIPT_LUA_USE_MT_LOCK)
	mutable Semaphore m_lock;
#endif
	ScriptContextLua* m_lockContext;
	std::vector< RegisteredClass > m_classRegistry;
	RefArray< ScriptContextLua > m_contexts;
	Ref< ScriptDebuggerLua > m_debugger;
	Ref< ScriptProfilerLua > m_profiler;
	float m_collectStepFrequency;
	int32_t m_collectSteps;
	float m_collectTargetSteps;
	size_t m_totalMemoryUse;
	size_t m_lastMemoryUse;

	void lock(ScriptContextLua* context)
	{
#if defined(T_SCRIPT_LUA_USE_MT_LOCK)
		m_lock.wait();
#endif
		m_lockContext = context;
	}

	void unlock()
	{
#if defined(T_SCRIPT_LUA_USE_MT_LOCK)
		m_lock.release();
#endif
	}

	void destroyContext(ScriptContextLua* context);

	void pushObject(ITypedObject* object);

	void pushAny(const Any& any);

	void pushAny(const Any* anys, int32_t count);

	Any toAny(int32_t index);

	void toAny(int32_t base, int32_t count, Any* outAnys);

	void collectGarbageFull();

	void collectGarbageFullNoLock();

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

	static int luaAllocatedMemory(lua_State* luaState);

	static int luaStrictEnvironment(lua_State* luaState);

	static void hookCallback(lua_State* L, lua_Debug* ar);

	static int luaPanic(lua_State* luaState);
};

	}
}

#endif	// traktor_script_ScriptManagerLua_H
