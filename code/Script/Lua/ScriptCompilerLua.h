#pragma once

#include "Script/IScriptCompiler.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_LUA_EXPORT) || defined(T_SCRIPT_LUAJIT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

struct lua_State;

namespace traktor
{
	namespace script
	{

/*! LUA script compiler.
 * \ingroup LUA Script
 */
class T_DLLCLASS ScriptCompilerLua : public IScriptCompiler
{
	T_RTTI_CLASS;

public:
	ScriptCompilerLua();

	virtual ~ScriptCompilerLua();

	virtual Ref< IScriptBlob > compile(const std::wstring& fileName, const std::wstring& script, IErrorCallback* errorCallback) const override final;

private:
	lua_State* m_luaState;
};

	}
}
