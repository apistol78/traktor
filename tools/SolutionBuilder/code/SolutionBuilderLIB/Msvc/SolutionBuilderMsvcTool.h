#ifndef SolutionBuilderMsvcTool_H
#define SolutionBuilderMsvcTool_H

#include <Core/Serialization/Serializable.h>
#include <Core/Io/OutputStream.h>

class GeneratorContext;
class Solution;
class Project;
class Configuration;

/*! \brief Visual Studio solution tool. */
class SolutionBuilderMsvcTool : public traktor::Serializable
{
	T_RTTI_CLASS(SolutionBuilderMsvcTool)

public:
	virtual bool generate(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		Configuration* configuration,
		traktor::OutputStream& os
	) const = 0;
};

#endif	// SolutionBuilderMsvcTool_H
