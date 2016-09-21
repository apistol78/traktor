#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Io/OutputStream.h>
#include <Core/Misc/String.h>
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCXClCompileBuildTool.h"
#include "SolutionBuilderLIB/Msvc/GeneratorContext.h"

using namespace traktor;

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"SolutionBuilderMsvcVCXClCompileBuildTool", 0, SolutionBuilderMsvcVCXClCompileBuildTool, SolutionBuilderMsvcVCXBuildTool)

bool SolutionBuilderMsvcVCXClCompileBuildTool::generateProject(
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

	if (!filter.empty())
	{
		os << L"<" << m_name << L" Include=\"" << fileName.getPathName() << L"\">" << Endl;
		os << L"\t<ObjectFileName>$(IntDir)\\" << filter << L"\\%(Filename).obj</ObjectFileName>" << Endl;
		os << L"</" << m_name << L">" << Endl;
	}
	else
		os << L"<" << m_name << L" Include=\"" << fileName.getPathName() << L"\" />" << Endl;

	return true;
}
