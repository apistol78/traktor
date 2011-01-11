#ifndef traktor_script_ScriptContextLua_H
#define traktor_script_ScriptContextLua_H

#include "Script/IScriptContext.h"

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

class IScriptClass;
class ScriptManagerLua;

/*! \brief LUA scripting context.
 * \ingroup LUA Script
 */
class T_DLLCLASS ScriptContextLua : public IScriptContext
{
	T_RTTI_CLASS;

public:
	ScriptContextLua(ScriptManagerLua* scriptManager, lua_State* luaState);

	virtual ~ScriptContextLua();

	virtual void destroy();

	virtual void setGlobal(const std::wstring& globalName, const Any& globalValue);

	virtual Any getGlobal(const std::wstring& globalName);

	virtual bool executeScript(const IScriptResource* scriptResource, const Guid& scriptGuid);

	virtual bool haveFunction(const std::wstring& functionName) const;

	virtual Any executeFunction(const std::wstring& functionName, uint32_t argc, const Any* argv);

	virtual Any executeMethod(Object* self, const std::wstring& methodName, uint32_t argc, const Any* argv);

private:
	Ref< ScriptManagerLua > m_scriptManager;
	lua_State* m_luaState;
	int32_t m_environmentRef;
};

	}
}

#endif	// traktor_script_ScriptContextLua_H
