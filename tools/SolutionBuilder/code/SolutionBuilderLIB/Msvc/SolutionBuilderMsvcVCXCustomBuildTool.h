#ifndef SolutionBuilderMsvcVCXCustomBuildTool_H
#define SolutionBuilderMsvcVCXCustomBuildTool_H

#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCXBuildTool.h"

class SolutionBuilderMsvcVCXCustomBuildTool : public SolutionBuilderMsvcVCXBuildTool
{
	T_RTTI_CLASS;

public:
	virtual bool generate(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		const traktor::Path& fileName,
		traktor::OutputStream& os
	) const;

	virtual bool serialize(traktor::ISerializer& s);

private:
	std::wstring m_command;
	std::wstring m_message;
	std::wstring m_outputs;
};

#endif	// SolutionBuilderMsvcVCXCustomBuildTool_H
