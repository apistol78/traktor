/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ScriptProcessor_H
#define ScriptProcessor_H

#include <Core/Ref.h>
#include <Script/IScriptManager.h>

class Project;
class Solution;

class ScriptProcessor : public traktor::Object
{
	T_RTTI_CLASS;

public:
	bool create();

	void destroy();

	bool generateFromFile(const Solution* solution, const Project* project, const std::wstring& projectPath, const std::wstring& fileName, std::wstring& output) const;

	bool generateFromSource(const Solution* solution, const Project* project, const std::wstring& projectPath, const std::wstring& source, std::wstring& output) const;

private:
	traktor::Ref< traktor::script::IScriptManager > m_scriptManager;
};

#endif	// ScriptProcessor_H
