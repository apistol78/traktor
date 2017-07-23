/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderMsvcVCXBuildTool_H
#define SolutionBuilderMsvcVCXBuildTool_H

#include <Core/Serialization/ISerializable.h>
#include <Core/Io/Path.h>

class GeneratorContext;
class Solution;
class Project;

class SolutionBuilderMsvcVCXBuildTool : public traktor::ISerializable
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

	virtual bool generateFilter(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		const std::wstring& filter,
		const traktor::Path& fileName,
		traktor::OutputStream& os
	) const;

	virtual void serialize(traktor::ISerializer& s);

protected:
	std::wstring m_name;
	std::wstring m_fileType;
};

#endif	// SolutionBuilderMsvcVCXBuildTool_H
