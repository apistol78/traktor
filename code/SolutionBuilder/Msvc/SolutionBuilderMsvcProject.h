#pragma once

#include <string>
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace sb
	{

class GeneratorContext;
class Solution;
class Project;

/*! Visual Studio solution project settings. */
class SolutionBuilderMsvcProject : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getPlatform() const = 0;

	virtual bool getInformation(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		std::wstring& outSolutionPath,
		std::wstring& outProjectPath,
		std::wstring& outProjectFileName,
		std::wstring& outProjectGuid
	) const = 0;

	virtual bool generate(
		GeneratorContext& context,
		Solution* solution,
		Project* project
	) const = 0;
};

	}
}

