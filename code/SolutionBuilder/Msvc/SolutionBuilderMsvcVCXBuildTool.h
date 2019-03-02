#pragma once

#include "Core/Serialization/ISerializable.h"

namespace traktor
{

class Path;

	namespace sb
	{

class GeneratorContext;
class Solution;
class Project;

class SolutionBuilderMsvcVCXBuildTool : public ISerializable
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
	) const;

	virtual bool generateFilter(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		const std::wstring& filter,
		const Path& fileName,
		OutputStream& os
	) const;

	virtual void serialize(ISerializer& s) override;

protected:
	std::wstring m_name;
	std::wstring m_fileType;
};

	}
}

