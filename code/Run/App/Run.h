/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Core/Object.h"
#include "Core/Class/Any.h"

namespace traktor
{

class Environment;

	namespace script
	{

class IScriptCompiler;
class IScriptContext;
class IScriptManager;

	}

	namespace run
	{

/*! Simplified interface for common scriptable actions.
 * \ingroup Run
 */
class Run : public Object
{
	T_RTTI_CLASS;

public:
	explicit Run(
		script::IScriptCompiler* scriptCompiler,
		script::IScriptManager* scriptManager,
		script::IScriptContext* scriptContext
	);

	void cd(const std::wstring& path);

	void pushd(const std::wstring& path);

	void popd();

	std::wstring cwd() const;

	int32_t run(const std::wstring& command, const std::wstring& saveOutputAs = L"", const Environment* env = nullptr);

	int32_t execute(const std::wstring& command, const std::wstring& saveOutputAs = L"", const Environment* env = nullptr, bool elevated = false);

	const std::wstring& stdOut() const;

	const std::wstring& stdErr() const;

	int32_t exitCode() const;

	bool exist(const std::wstring& path);

	bool rm(const std::wstring& path);

	bool copy(const std::wstring& source, const std::wstring& target);

	bool replace(const std::wstring& source, const std::wstring& target);

	bool mkdir(const std::wstring& path);

	bool rmdir(const std::wstring& path);

	void sleep(int32_t ms);

	Any getProperty(const std::wstring& fileName1, const std::wstring& fileName2, const std::wstring& propertyName, const Any& defaultValue) const;

	bool setProperty(const std::wstring& fileName, const std::wstring& propertyName, const Any& value) const;

	std::wstring resolve(const std::wstring& text);

	bool loadModule(const std::wstring& moduleName);

	bool loadScript(const std::wstring& fileName);

	std::wstring evaluate(const std::wstring& fileName);

	static void registerRuntimeClasses(script::IScriptManager* scriptManager);

private:
	Ref< script::IScriptCompiler > m_scriptCompiler;
	Ref< script::IScriptManager > m_scriptManager;
	Ref< script::IScriptContext > m_scriptContext;
	std::list< std::wstring > m_cwd;
	std::wstring m_stdOut;
	std::wstring m_stdErr;
	int32_t m_exitCode;
};

	}
}

