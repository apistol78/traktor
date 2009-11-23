#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Io/OutputStream.h>
#include <Core/Misc/String.h>
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCXCustomBuildTool.h"
#include "SolutionBuilderLIB/Msvc/GeneratorContext.h"

using namespace traktor;

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"SolutionBuilderMsvcVCXCustomBuildTool", 0, SolutionBuilderMsvcVCXCustomBuildTool, SolutionBuilderMsvcVCXBuildTool)

bool SolutionBuilderMsvcVCXCustomBuildTool::generate(
	GeneratorContext& context,
	Solution* solution,
	Project* project,
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

bool SolutionBuilderMsvcVCXCustomBuildTool::serialize(ISerializer& s)
{
	if (!SolutionBuilderMsvcVCXBuildTool::serialize(s))
		return false;

	s >> Member< std::wstring >(L"command", m_command);
	s >> Member< std::wstring >(L"message", m_message);
	s >> Member< std::wstring >(L"outputs", m_outputs);

	return true;
}
