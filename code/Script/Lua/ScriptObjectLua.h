#ifndef traktor_script_ScriptObjectLua_H
#define traktor_script_ScriptObjectLua_H

#include "Core/Class/IRuntimeObject.h"

struct lua_State;

namespace traktor
{
	namespace script
	{

class ScriptClassLua;

class ScriptObjectLua : public IRuntimeObject
{
	T_RTTI_CLASS;

public:
	ScriptObjectLua(lua_State*& luaState, int32_t tableRef, const ScriptClassLua* scriptClass);

	virtual ~ScriptObjectLua();

	virtual const IRuntimeClass* getRuntimeClass() const;

	void push();

private:
	lua_State*& m_luaState;
	int32_t m_tableRef;
	const ScriptClassLua* m_scriptClass;
};

	}
}

#endif	// traktor_script_ScriptObjectLua_H
