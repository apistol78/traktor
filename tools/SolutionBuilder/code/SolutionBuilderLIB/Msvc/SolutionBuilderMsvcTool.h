/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderMsvcTool_H
#define SolutionBuilderMsvcTool_H

#include <Core/Serialization/ISerializable.h>
#include <Core/Io/OutputStream.h>

class GeneratorContext;
class Solution;
class Project;
class Configuration;

/*! \brief Visual Studio solution tool. */
class SolutionBuilderMsvcTool : public traktor::ISerializable
{
	T_RTTI_CLASS;

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
