/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Class/Any.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Object.h"

#include <list>

namespace traktor
{

class Environment;
class IRuntimeClassRegistrar;
class IRuntimeDelegate;

}

namespace traktor::script
{

class IScriptCompiler;
class IScriptContext;
class IScriptManager;

}

namespace traktor::run
{

/*! Simplified interface for common script actions.
 * \ingroup Run
 */
class Run : public Object
{
	T_RTTI_CLASS;

public:
	constexpr static int32_t TraverseAny = 0;
	constexpr static int32_t TraverseBreadthFirst = 1;
	constexpr static int32_t TraverseDepthFirst = 2;

	explicit Run(
		script::IScriptCompiler* scriptCompiler,
		script::IScriptManager* scriptManager,
		script::IScriptContext* scriptContext,
		const CommandLine& commandLine);

	void cd(const std::wstring& path);

	void pushd(const std::wstring& path);

	void popd();

	std::wstring cwd() const;

	int32_t run(const std::wstring& command, const std::wstring& saveOutputAs = L"", const Environment* env = nullptr);

	int32_t execute(const std::wstring& command, const std::wstring& saveOutputAs = L"", const Environment* env = nullptr, bool elevated = false);

	const std::wstring& stdOut() const;

	const std::wstring& stdErr() const;

	const CommandLine& commandLine() const;

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

	bool directory(const std::wstring& path, IRuntimeDelegate* delegate);

	bool traverseDirectories(const std::wstring& path, int32_t mode, IRuntimeDelegate* delegate);

	static void registerRuntimeClasses(IRuntimeClassRegistrar* runtimeRegistrar);

private:
	Ref< script::IScriptCompiler > m_scriptCompiler;
	Ref< script::IScriptManager > m_scriptManager;
	Ref< script::IScriptContext > m_scriptContext;
	CommandLine m_commandLine;
	std::list< std::wstring > m_cwd;
	std::wstring m_stdOut;
	std::wstring m_stdErr;
	int32_t m_exitCode;
};

}
