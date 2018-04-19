/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderMsvcVCProj_H
#define SolutionBuilderMsvcVCProj_H

#include "SolutionBuilder/Msvc/SolutionBuilderMsvcProject.h"

namespace traktor
{
	namespace sb
	{

class SolutionBuilderMsvcConfiguration;
class SolutionBuilderMsvcTool;
class ProjectItem;

/*! \brief Visual Studio solution project settings. */
class SolutionBuilderMsvcVCProj : public SolutionBuilderMsvcProject
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getPlatform() const T_OVERRIDE T_FINAL;

	virtual bool getInformation(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		std::wstring& outSolutionPath,
		std::wstring& outProjectPath,
		std::wstring& outProjectFileName,
		std::wstring& outProjectGuid
	) const T_OVERRIDE T_FINAL;

	virtual bool generate(
		GeneratorContext& context,
		Solution* solution,
		Project* project
	) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::map< std::wstring, std::wstring > m_staticOptions;
	std::wstring m_platform;
	Ref< SolutionBuilderMsvcConfiguration > m_configurations[4];	//< One array per target type (executable, console, static library, shared library).
	std::map< std::wstring, Ref< SolutionBuilderMsvcTool > > m_customTools;

	bool addItem(GeneratorContext& context, Solution* solution, Project* project, ProjectItem* item, OutputStream& os) const;
};

	}
}

#endif	// SolutionBuilderMsvcVCProj_H
