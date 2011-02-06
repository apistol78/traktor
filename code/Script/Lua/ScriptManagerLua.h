#ifndef traktor_script_ScriptManagerLua_H
#define traktor_script_ScriptManagerLua_H

#include <csetjmp>
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Thread/Semaphore.h"
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

	virtual Ref< IScriptResource > compile(const std::wstring& script, bool strip, IErrorCallback* errorCallback) const;

	virtual Ref< IScriptContext > createContext();

	virtual Ref< IScriptDebugger > createDebugger();

private:
	friend class ScriptContextLua;

	struct RegisteredClass
	{
		Ref< IScriptClass > scriptClass;
		int32_t metaTableRef;
	};

	lua_State* m_luaState;
	int32_t m_gcMetaRef;
	Semaphore m_lock;
	ScriptContextLua* m_currentContext;
	std::vector< RegisteredClass > m_classRegistry;
	SmallMap< const TypeInfo*, uint32_t > m_classRegistryLookup;
	static std::jmp_buf ms_jmpbuf;

	void lock(ScriptContextLua* context);

	void unlock();

	void pushObject(Object* object);

	void pushAny(const Any& any);

	Any toAny(int32_t index);

	bool setPanicJump();

	static int classIndexLookup(lua_State* luaState);

	static int classCallConstructor(lua_State* luaState);

	static int classCallMethod(lua_State* luaState);

	static int classCallUnknownMethod(lua_State* luaState);

	static int classGcMethod(lua_State* luaState);

	static int classEqualMethod(lua_State* luaState);

	static int luaPanic(lua_State* luaState);
};

	}
}

#endif	// traktor_script_ScriptManagerLua_H
