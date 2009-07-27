#include <Core/Serialization/Serializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Io/OutputStream.h>
#include <Core/Misc/String.h>
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCXBuildTool.h"

using namespace traktor;

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"SolutionBuilderMsvcVCXBuildTool", SolutionBuilderMsvcVCXBuildTool, Serializable)

bool SolutionBuilderMsvcVCXBuildTool::generate(
	GeneratorContext& context,
	Solution* solution,
	Project* project,
	const Path& fileName,
	OutputStream& os
) const
{
	if (compareIgnoreCase(m_fileType, fileName.getExtension()) != 0)
		return true;

	os << L"<" << m_name << L" Include=\"" << fileName.getPathName() << L"\" />" << Endl;
	return true;
}

bool SolutionBuilderMsvcVCXBuildTool::serialize(Serializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< std::wstring >(L"fileType", m_fileType);
	return true;
}
