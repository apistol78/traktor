#pragma once

#include "Core/Serialization/ISerializable.h"

namespace traktor
{

class OutputStream;

	namespace sb
	{

class GeneratorContext;
class Solution;
class Project;
class Configuration;

/*! \brief Visual Studio solution tool. */
class SolutionBuilderMsvcTool : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual bool generate(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		Configuration* configuration,
		OutputStream& os
	) const = 0;
};

	}
}

