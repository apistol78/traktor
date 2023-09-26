/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#define T_SCRIPT_LUA_USE_MT_LOCK

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
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

}

namespace traktor::script
{

class ScriptContextLua;
class ScriptDebuggerLua;
class ScriptProfilerLua;

/*! LUA script manager.
 * \ingroup LUA Script
 */
class T_DLLCLASS ScriptManagerLua : public IScriptManager
{
	T_RTTI_CLASS;

public:
	ScriptManagerLua();

	virtual ~ScriptManagerLua();

	virtual void destroy() override final;

	virtual void registerClass(IRuntimeClass* runtimeClass) override final;

	virtual Ref< IScriptContext > createContext(bool strict) override final;

	virtual Ref< IScriptDebugger > createDebugger() override final;

	virtual Ref< IScriptProfiler > createProfiler() override final;

	virtual void collectGarbage(bool full) override final;

	virtual void getStatistics(ScriptStatistics& outStatistics) const override final;

	void pushObject(ITypedObject* object);

	void pushAny(const Any& any);

	void pushAny(const Any* anys, int32_t count);

	Any toAny(int32_t index);

	void toAny(int32_t base, int32_t count, Any* outAnys);

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

private:
	friend class ScriptContextLua;
	friend class ScriptDebuggerLua;
	friend class ScriptProfilerLua;

	struct RegisteredClass
	{
		Ref< const IRuntimeClass > runtimeClass;
		int32_t classTableRef;
	};

	lua_State* m_luaState;
	void* m_defaultAllocFn;
	void* m_defaultAllocOpaque;
	int32_t m_objectTableRef;
#if defined(T_SCRIPT_LUA_USE_MT_LOCK)
	mutable Semaphore m_lock;
#endif
	static ScriptManagerLua* ms_instance;
	ScriptContextLua* m_lockContext;
	AlignedVector< RegisteredClass > m_classRegistry;
	RefArray< ScriptContextLua > m_contexts;
	Ref< ScriptDebuggerLua > m_debugger;
	Ref< ScriptProfilerLua > m_profiler;
	float m_collectStepFrequency;
	int32_t m_collectSteps;
	float m_collectTargetSteps;
	size_t m_totalMemoryUse;
	size_t m_lastMemoryUse;

	void destroyContext(ScriptContextLua* context);

	void collectGarbageFull();

	void collectGarbageFullNoLock();

	void collectGarbagePartial();

	void breakDebugger(lua_State* luaState);

	static int classGc(lua_State* luaState);

	static int classNew(lua_State* luaState);

	static int classCallUnknownMethod(lua_State* luaState);

	static int classCallMethod(lua_State* luaState);

	static int classCallStaticMethod(lua_State* luaState);

	static int classSetProperty(lua_State* luaState);

	static int classGetProperty(lua_State* luaState);

	static int classEqual(lua_State* luaState);

	static int classAdd(lua_State* luaState);

	static int classSubtract(lua_State* luaState);

	static int classMultiply(lua_State* luaState);

	static int classDivide(lua_State* luaState);

	static int classNewIndex(lua_State* luaState);

	static int classIndex(lua_State* luaState);

	static void* luaAlloc(void* ud, void* ptr, size_t osize, size_t nsize);

	static int luaAllocatedMemory(lua_State* luaState);

	static int luaStrictEnvironment(lua_State* luaState);

	static void hookCallback(lua_State* L, lua_Debug* ar);

	static int luaPanic(lua_State* luaState);
};

}
