/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ScriptProcessor_H
#define ScriptProcessor_H

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

	bool generateFromFile(const Solution* solution, const Project* project, const std::wstring& projectPath, const std::wstring& fileName, std::wstring& output) const;

	bool generateFromSource(const Solution* solution, const Project* project, const std::wstring& projectPath, const std::wstring& source, std::wstring& output) const;

private:
	Ref< script::IScriptManager > m_scriptManager;
};

	}
}

#endif	// ScriptProcessor_H
