/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberStl.h>
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcGenericTool.h"
#include "SolutionBuilderLIB/Msvc/GeneratorContext.h"

using namespace traktor;

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"SolutionBuilderMsvcGenericTool", 0, SolutionBuilderMsvcGenericTool, SolutionBuilderMsvcTool)

bool SolutionBuilderMsvcGenericTool::generate(GeneratorContext& context, Solution* solution, Project* project, Configuration* configuration, traktor::OutputStream& os) const
{
	os << L"<Tool" << Endl;
	os << IncreaseIndent;
	os << L"Name=\"" << m_toolName << L"\"" << Endl;

	for (std::map< std::wstring, std::wstring >::const_iterator i = m_staticOptions.begin(); i != m_staticOptions.end(); ++i)
		os << i->first << L"=\"" << context.format(i->second) << L"\"" << Endl;

	os << DecreaseIndent;
	os << L"/>" << Endl;
	return true;
}

void SolutionBuilderMsvcGenericTool::serialize(traktor::ISerializer& s)
{
	s >> Member< std::wstring >(L"toolName", m_toolName);
	s >> MemberStlMap< std::wstring, std::wstring >(L"staticOptions", m_staticOptions);
}
