/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderMsvcProject_H
#define SolutionBuilderMsvcProject_H

#include <Core/Serialization/ISerializable.h>

class GeneratorContext;
class Solution;
class Project;

/*! \brief Visual Studio solution project settings. */
class SolutionBuilderMsvcProject : public traktor::ISerializable
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getPlatform() const = 0;

	virtual bool getInformation(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
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

#endif	// SolutionBuilderMsvcProject_H
