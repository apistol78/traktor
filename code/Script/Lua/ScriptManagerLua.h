#ifndef traktor_script_ScriptManagerLua_H
#define traktor_script_ScriptManagerLua_H

#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Timer/Timer.h"
#include "Script/IScriptManager.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_LUA_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

struct lua_State;

namespace traktor
{
	namespace script
	{

class Any;
class ScriptContextLua;
class ScriptDebuggerLua;

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

	virtual void collectGarbage(bool full);

	virtual void getStatistics(ScriptStatistics& outStatistics) const;

private:
	friend class ScriptContextLua;
	friend class ScriptDebuggerLua;
	friend class ScriptDelegateLua;

	struct RegisteredClass
	{
		Ref< IScriptClass > scriptClass;
		int32_t metaTableRef;
	};

	lua_State* m_luaState;
	int32_t m_objectTableRef;
	mutable Semaphore m_lock;
	std::vector< RegisteredClass > m_classRegistry;
	SmallMap< const TypeInfo*, uint32_t > m_classRegistryLookup;
	RefArray< ScriptContextLua > m_contexts;
	ScriptContextLua* m_lockContext;
	Ref< ScriptDebuggerLua > m_debugger;
	Timer m_timer;
	float m_collectStepFrequency;
	int32_t m_collectSteps;
	float m_collectTargetSteps;
	size_t m_totalMemoryUse;
	size_t m_lastMemoryUse;

	void destroyContext(ScriptContextLua* context);

	void lock(ScriptContextLua* context);

	void unlock();

	void pushObject(Object* object);

	void pushAny(const Any& any);

	Any toAny(int32_t index);

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

	static int luaPanic(lua_State* luaState);

	static int luaDumpWriter(lua_State* luaState, const void* p, size_t sz, void* ud);

	static const char* luaDumpReader(lua_State* luaState, void* data, size_t* size);
};

	}
}

#endif	// traktor_script_ScriptManagerLua_H
