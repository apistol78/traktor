/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_script_ScriptObjectLua_H
#define traktor_script_ScriptObjectLua_H

#include "Core/Object.h"

struct lua_State;

namespace traktor
{
	namespace script
	{

class ScriptObjectLua : public Object
{
	T_RTTI_CLASS;

public:
	ScriptObjectLua(lua_State*& luaState, int32_t tableRef);

	virtual ~ScriptObjectLua();

	void push();

private:
	lua_State*& m_luaState;
	int32_t m_tableRef;
};

	}
}

#endif	// traktor_script_ScriptObjectLua_H
