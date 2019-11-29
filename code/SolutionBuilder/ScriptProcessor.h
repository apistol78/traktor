#pragma once

#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOLUTIONBUILDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

class IScriptCompiler;
class IScriptContext;
class IScriptManager;

	}

	namespace sb
	{

class Project;
class Solution;

class T_DLLCLASS ScriptProcessor : public Object
{
	T_RTTI_CLASS;

public:
	bool create();

	void destroy();

	bool prepare(const std::wstring& fileName);

	bool generate(const Solution* solution, const Project* project, const std::wstring& projectPath, std::wstring& output) const;

private:
	Ref< script::IScriptCompiler > m_scriptCompiler;
	Ref< script::IScriptManager > m_scriptManager;
	Ref< script::IScriptContext > m_scriptContext;
	std::vector< std::wstring > m_sections;
};

	}
}

