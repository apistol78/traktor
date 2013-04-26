#ifndef traktor_script_ScriptContextLua_H
#define traktor_script_ScriptContextLua_H

#include "Script/IScriptContext.h"
#include "Script/Types.h"

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
class ScriptContextLua : public IScriptContext
{
	T_RTTI_CLASS;

public:
	virtual ~ScriptContextLua();

	virtual void destroy();

	virtual void setGlobal(const std::string& globalName, const Any& globalValue);

	virtual Any getGlobal(const std::string& globalName);

	virtual bool haveFunction(const std::string& functionName) const;

	virtual Any executeFunction(const std::string& functionName, uint32_t argc, const Any* argv);

	virtual Any executeMethod(Object* self, const std::string& methodName, uint32_t argc, const Any* argv);

private:
	friend class ScriptDebuggerLua;
	friend class ScriptManagerLua;

	ScriptManagerLua* m_scriptManager;
	lua_State* m_luaState;
	int32_t m_environmentRef;
	source_map_t m_map;

	ScriptContextLua(ScriptManagerLua* scriptManager, lua_State* luaState, int32_t environmentRef, const source_map_t& map);

	static int32_t runtimeError(lua_State* luaState);
};

	}
}

#endif	// traktor_script_ScriptContextLua_H
