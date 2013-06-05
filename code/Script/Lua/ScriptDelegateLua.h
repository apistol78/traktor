#ifndef traktor_script_ScriptDelegateLua_H
#define traktor_script_ScriptDelegateLua_H

#include "Script/IScriptDelegate.h"

struct lua_State;

namespace traktor
{
	namespace script
	{

class ScriptContextLua;
class ScriptManagerLua;

class ScriptDelegateLua : public IScriptDelegate
{
	T_RTTI_CLASS;

public:
	ScriptDelegateLua(ScriptManagerLua* manager, ScriptContextLua* context, lua_State*& luaState);

	virtual ~ScriptDelegateLua();

	void push();

	virtual Any call(int32_t argc, const Any* argv);

private:
	ScriptManagerLua* m_manager;
	ScriptContextLua* m_context;
	lua_State*& m_luaState;
	int32_t m_functionRef;
};

	}
}

#endif	// traktor_script_ScriptDelegateLua_H
