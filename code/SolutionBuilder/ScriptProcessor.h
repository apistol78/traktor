/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

#include <string>
#include <vector>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOLUTIONBUILDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class CommandLine;

}

namespace traktor::script
{

class IScriptCompiler;
class IScriptContext;
class IScriptManager;

}

namespace traktor::sb
{

class Project;
class Solution;

class T_DLLCLASS ScriptProcessor : public Object
{
	T_RTTI_CLASS;

public:
	bool create(const CommandLine& cmdLine);

	void destroy();

	bool prepare(const std::wstring& fileName);

	bool generate(const Solution* solution, const Project* project, const std::wstring& configurationName, const std::wstring& projectPath, std::wstring& output) const;

private:
	Ref< script::IScriptCompiler > m_scriptCompiler;
	Ref< script::IScriptManager > m_scriptManager;
	Ref< script::IScriptContext > m_scriptContext;
	std::vector< std::wstring > m_sections;
	bool m_profile = false;
};

}
