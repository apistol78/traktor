#ifndef SolutionBuilderMsvcVCXProjVGDB_H
#define SolutionBuilderMsvcVCXProjVGDB_H

#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCXProj.h"

class VGDBCredentials;

/*! \brief Visual Studio solution project (including VisualGDB) settings. */
class SolutionBuilderMsvcVCXProjVGDB : public SolutionBuilderMsvcVCXProj
{
	T_RTTI_CLASS;

public:
	SolutionBuilderMsvcVCXProjVGDB();

	virtual bool generate(
		GeneratorContext& context,
		Solution* solution,
		Project* project
	) const;

private:
	bool generateProject(
		GeneratorContext& context,
		Solution* solution,
		Project* project
	) const;

	bool generateMakefiles(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		const VGDBCredentials* credentials
	) const;

	bool collectFiles(
		Project* project,
		ProjectItem* item,
		std::vector< traktor::Path >& outFiles
	) const;

	void collectLinkDependencies(
		Solution* solution,
		Project* project,
		const std::wstring& configurationName,
		std::vector< std::wstring >& outLibraries
	) const;
};

#endif	// SolutionBuilderMsvcVCXProjVGDB_H
