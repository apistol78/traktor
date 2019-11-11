#include "Core/Io/OutputStream.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcLibrarianTool.h"
#include "SolutionBuilder/Msvc/GeneratorContext.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/Configuration.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"SolutionBuilderMsvcLibrarianTool", 0, SolutionBuilderMsvcLibrarianTool, SolutionBuilderMsvcTool)

bool SolutionBuilderMsvcLibrarianTool::generate(GeneratorContext& context, Solution* solution, Project* project, Configuration* configuration, OutputStream& os) const
{
	os << L"<Tool" << Endl;
	os << IncreaseIndent;
	os << L"Name=\"VCLibrarianTool\"" << Endl;

	// Output file.
	os << L"OutputFile=\"$(OutDir)/" << project->getName() << L".lib\"" << Endl;

	// Static options.
	for (const auto& it : m_staticOptions)
		os << it.first << L"=\"" << context.format(it.second) << L"\"" << Endl;

	os << DecreaseIndent;
	os << L"/>" << Endl;
	return true;
}

void SolutionBuilderMsvcLibrarianTool::serialize(ISerializer& s)
{
	s >> MemberStlMap< std::wstring, std::wstring >(L"staticOptions", m_staticOptions);
}

	}
}
