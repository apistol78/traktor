#pragma once

#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXBuildTool.h"

namespace traktor
{
	namespace sb
	{

class SolutionBuilderMsvcVCXClCompileBuildTool : public SolutionBuilderMsvcVCXBuildTool
{
	T_RTTI_CLASS;

public:
	virtual bool generateProject(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		const std::wstring& filter,
		const Path& fileName,
		OutputStream& os
	) const override final;
};

	}
}

