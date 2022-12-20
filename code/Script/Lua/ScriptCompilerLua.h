/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Thread/Semaphore.h"
#include "Script/IScriptCompiler.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_LUA_EXPORT) || defined(T_SCRIPT_LUAJIT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

struct lua_State;

namespace traktor::script
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
	mutable Semaphore m_lock;
	lua_State* m_luaState;
};

}
