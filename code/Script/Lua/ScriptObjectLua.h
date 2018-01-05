/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_script_ScriptObjectLua_H
#define traktor_script_ScriptObjectLua_H

#include "Core/Class/IRuntimeObject.h"

struct lua_State;

namespace traktor
{
	namespace script
	{

class ScriptObjectLua : public IRuntimeObject
{
	T_RTTI_CLASS;

public:
	ScriptObjectLua(ScriptManagerLua* scriptManager, ScriptContextLua* scriptContext, lua_State*& luaState, int32_t tableRef);

	virtual ~ScriptObjectLua();

	virtual Ref< const IRuntimeClass > getRuntimeClass() const T_OVERRIDE T_FINAL;

	void push() const;

private:
	ScriptManagerLua* m_scriptManager;
	ScriptContextLua* m_scriptContext;
	lua_State*& m_luaState;
	int32_t m_tableRef;
};

	}
}

#endif	// traktor_script_ScriptObjectLua_H
