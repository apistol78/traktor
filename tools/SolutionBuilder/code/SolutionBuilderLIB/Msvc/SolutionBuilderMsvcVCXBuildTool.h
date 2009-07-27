#ifndef SolutionBuilderMsvcVCXBuildTool_H
#define SolutionBuilderMsvcVCXBuildTool_H

#include <Core/Serialization/Serializable.h>
#include <Core/Io/Path.h>

class GeneratorContext;
class Solution;
class Project;

class SolutionBuilderMsvcVCXBuildTool : public traktor::Serializable
{
	T_RTTI_CLASS(SolutionBuilderMsvcVCXBuildTool)

public:
	virtual bool generate(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		const traktor::Path& fileName,
		traktor::OutputStream& os
	) const;

	virtual bool serialize(traktor::Serializer& s);

protected:
	std::wstring m_name;
	std::wstring m_fileType;
};

#endif	// SolutionBuilderMsvcVCXBuildTool_H
