/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Acquire.h"
#include "Script/IErrorCallback.h"
#include "Script/Lua/ScriptBlobLua.h"
#include "Script/Lua/ScriptCompilerLua.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor
{
    namespace script
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptCompilerLua", 0, ScriptCompilerLua, IScriptCompiler)

ScriptCompilerLua::ScriptCompilerLua()
:	m_luaState(nullptr)
{
	m_luaState = luaL_newstate();
}

ScriptCompilerLua::~ScriptCompilerLua()
{
    lua_close(m_luaState);
}

Ref< IScriptBlob > ScriptCompilerLua::compile(const std::wstring& fileName, const std::wstring& script, IErrorCallback* errorCallback) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	std::string metaFileName = "@" + wstombs(Utf8Encoding(), fileName);
	std::string text = wstombs(Utf8Encoding(), script);

	int32_t result = luaL_loadbuffer(
		m_luaState,
		text.c_str(),
		text.length(),
		metaFileName.c_str()
	);
	if (result != 0)
	{
		std::wstring error = mbstows(lua_tostring(m_luaState, -1));

		size_t p0 = error.find(L':');
		T_ASSERT(p0 != error.npos);

		error = error.substr(p0 + 1);

		size_t p1 = error.find(L':');
		T_ASSERT(p1 != error.npos);

		int32_t line = parseString< int32_t >(error.substr(0, p1));
		error = trim(error.substr(p1 + 1));

		if (errorCallback)
			errorCallback->syntaxError(fileName, line, error);
		else
			log::error << fileName << L" (" << line << L"): " << error << Endl;

		lua_pop(m_luaState, 1);
		return nullptr;
	}

	lua_pop(m_luaState, 1);

	Ref< ScriptBlobLua > blob = new ScriptBlobLua();
	blob->m_fileName = wstombs(fileName);
	blob->m_script = text;

	return blob;
}

    }
}