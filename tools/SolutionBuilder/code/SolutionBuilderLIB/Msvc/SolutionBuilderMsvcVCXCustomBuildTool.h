#ifndef SolutionBuilderMsvcVCXCustomBuildTool_H
#define SolutionBuilderMsvcVCXCustomBuildTool_H

#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCXBuildTool.h"

class SolutionBuilderMsvcVCXCustomBuildTool : public SolutionBuilderMsvcVCXBuildTool
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

	virtual void serialize(traktor::ISerializer& s);

private:
	std::wstring m_command;
	std::wstring m_message;
	std::wstring m_outputs;
};

#endif	// SolutionBuilderMsvcVCXCustomBuildTool_H
