/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Io/OutputStream.h"
#include "Core/Misc/String.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXCustomBuildTool.h"
#include "SolutionBuilder/Msvc/GeneratorContext.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"SolutionBuilderMsvcVCXCustomBuildTool", 0, SolutionBuilderMsvcVCXCustomBuildTool, SolutionBuilderMsvcVCXBuildTool)

bool SolutionBuilderMsvcVCXCustomBuildTool::generateProject(
	GeneratorContext& context,
	Solution* solution,
	Project* project,
	const std::wstring& filter,
	const Path& fileName,
	OutputStream& os
) const
{
	if (compareIgnoreCase(m_fileType, fileName.getExtension()) != 0)
		return true;

	os << L"<" << m_name << L" Include=\"" << fileName.getPathName() << L"\">" << Endl;
	os << IncreaseIndent;

	os << L"<Command>" << context.format(m_command) << L"</Command>" << Endl;
	os << L"<Message>" << context.format(m_message) << L"</Message>" << Endl;
	os << L"<Outputs>" << context.format(m_outputs) << L"</Outputs>" << Endl;

	os << DecreaseIndent;
	os << L"</" << m_name << L">" << Endl;

	return true;
}

void SolutionBuilderMsvcVCXCustomBuildTool::serialize(ISerializer& s)
{
	SolutionBuilderMsvcVCXBuildTool::serialize(s);
	s >> Member< std::wstring >(L"command", m_command);
	s >> Member< std::wstring >(L"message", m_message);
	s >> Member< std::wstring >(L"outputs", m_outputs);
}

	}
}
