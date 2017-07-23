/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Io/OutputStream.h>
#include <Core/Misc/String.h>
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCXBuildTool.h"

using namespace traktor;

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"SolutionBuilderMsvcVCXBuildTool", 0, SolutionBuilderMsvcVCXBuildTool, ISerializable)

bool SolutionBuilderMsvcVCXBuildTool::generateProject(
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

	os << L"<" << m_name << L" Include=\"" << fileName.getPathName() << L"\" />" << Endl;
	return true;
}

bool SolutionBuilderMsvcVCXBuildTool::generateFilter(
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

	if (filter.empty())
		os << L"<" << m_name << L" Include=\"" << fileName.getPathName() << L"\" />" << Endl;
	else
	{
		os << L"<" << m_name << L" Include=\"" << fileName.getPathName() << L"\">" << Endl;
		os << L"\t<Filter>" << filter << L"</Filter>" << Endl;
		os << L"</" << m_name << L">" << Endl;
	}

	return true;
}

void SolutionBuilderMsvcVCXBuildTool::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< std::wstring >(L"fileType", m_fileType);
}
