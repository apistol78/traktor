/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderMsvcTool_H
#define SolutionBuilderMsvcTool_H

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

#endif	// SolutionBuilderMsvcTool_H
