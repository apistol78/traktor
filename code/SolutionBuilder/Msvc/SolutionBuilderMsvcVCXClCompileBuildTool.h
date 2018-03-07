/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderMsvcVCXClCompileBuildTool_H
#define SolutionBuilderMsvcVCXClCompileBuildTool_H

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
	) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// SolutionBuilderMsvcVCXClCompileBuildTool_H
