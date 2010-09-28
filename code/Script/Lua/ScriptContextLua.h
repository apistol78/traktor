#ifndef traktor_script_ScriptContextLua_H
#define traktor_script_ScriptContextLua_H

#include <vector>
#include "Core/RefArray.h"
#include "Core/Thread/Semaphore.h"
#include "Script/IScriptContext.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_LUA_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

struct lua_State;

namespace traktor
{
	namespace script
	{

class IScriptClass;

/*! \brief LUA scripting context.
 * \ingroup LUA Script
 */
class T_DLLCLASS ScriptContextLua : public IScriptContext
{
	T_RTTI_CLASS;

public:
	ScriptContextLua(const RefArray< IScriptClass >& registeredClasses);

	virtual ~ScriptContextLua();

	virtual void setGlobal(const std::wstring& globalName, const Any& globalValue);

	virtual Any getGlobal(const std::wstring& globalName);

	virtual bool executeScript(const std::wstring& script, bool compileOnly, IErrorCallback* errorCallback);

	virtual bool haveFunction(const std::wstring& functionName) const;

	virtual Any executeFunction(const std::wstring& functionName, uint32_t argc, const Any* argv);

	virtual Any executeMethod(Object* self, const std::wstring& methodName, uint32_t argc, const Any* argv);

private:
	struct RegisteredClass
	{
		Ref< IScriptClass > scriptClass;
		int metaTableRef;
	};

	mutable Semaphore m_lock;
	lua_State* m_luaState;
	std::vector< RegisteredClass > m_classRegistry;
	int32_t m_pending;

	void registerClass(IScriptClass* scriptClass);

	void pushAny(const Any& any);

	Any toAny(int32_t index);

	static int classIndexLookup(lua_State* luaState);

	static int callConstructor(lua_State* luaState);

	static int callMethod(lua_State* luaState);

	static int callUnknownMethod(lua_State* luaState);

	static int callProperty(lua_State* luaState);

	static int gcMethod(lua_State* luaState);

	static int luaPanic(lua_State* luaState);
};

	}
}

#endif	// traktor_script_ScriptContextLua_H
