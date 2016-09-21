#ifndef SolutionBuilderMsvcVCXClCompileBuildTool_H
#define SolutionBuilderMsvcVCXClCompileBuildTool_H

#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCXBuildTool.h"

class SolutionBuilderMsvcVCXClCompileBuildTool : public SolutionBuilderMsvcVCXBuildTool
{
	T_RTTI_CLASS;

public:
	virtual bool generateProject(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		const std::wstring& filter,
		const traktor::Path& fileName,
		traktor::OutputStream& os
	) const;
};

#endif	// SolutionBuilderMsvcVCXClCompileBuildTool_H
