#include "Core/Io/OutputStream.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcGenericTool.h"
#include "SolutionBuilder/Msvc/GeneratorContext.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"SolutionBuilderMsvcGenericTool", 0, SolutionBuilderMsvcGenericTool, SolutionBuilderMsvcTool)

bool SolutionBuilderMsvcGenericTool::generate(GeneratorContext& context, Solution* solution, Project* project, Configuration* configuration, OutputStream& os) const
{
	os << L"<Tool" << Endl;
	os << IncreaseIndent;
	os << L"Name=\"" << m_toolName << L"\"" << Endl;

	for (const auto& it : m_staticOptions)
		os << it.first << L"=\"" << context.format(it.second) << L"\"" << Endl;

	os << DecreaseIndent;
	os << L"/>" << Endl;
	return true;
}

void SolutionBuilderMsvcGenericTool::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"toolName", m_toolName);
	s >> MemberStlMap< std::wstring, std::wstring >(L"staticOptions", m_staticOptions);
}

	}
}
