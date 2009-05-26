#ifndef traktor_script_ScriptContextLua_H
#define traktor_script_ScriptContextLua_H

#include <vector>
#include "Script/ScriptContext.h"
#include "Core/Heap/Ref.h"

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

class ScriptClass;

/*! \brief LUA scripting context.
 * \ingroup LUA Script
 */
class T_DLLCLASS ScriptContextLua : public ScriptContext
{
	T_RTTI_CLASS(ScriptContextLua)

public:
	ScriptContextLua(const RefArray< ScriptClass >& registeredClasses);

	virtual ~ScriptContextLua();

	virtual void setGlobal(const std::wstring& globalName, const Any& globalValue);

	virtual bool executeScript(const std::wstring& script, bool compileOnly, IErrorCallback* errorCallback);

	virtual bool haveFunction(const std::wstring& functionName) const;

	virtual Any executeFunction(const std::wstring& functionName, const std::vector< Any >& arguments);

private:
	struct RegisteredClass
	{
		Ref< ScriptClass > scriptClass;
		int metaTableRef;
	};

	lua_State* m_luaState;
	std::vector< RegisteredClass > m_classRegistry;

	void registerClass(ScriptClass* scriptClass);

	void pushAny(const Any& any);

	Any toAny(int32_t index);

	static int callMethod(lua_State* luaState);

	static int gcMethod(lua_State* luaState);

	static int luaPanic(lua_State* luaState);
};

	}
}

#endif	// traktor_script_ScriptContextLua_H
