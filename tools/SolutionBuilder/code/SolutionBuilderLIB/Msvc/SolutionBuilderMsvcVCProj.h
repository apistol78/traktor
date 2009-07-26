#ifndef SolutionBuilderMsvcVCProj_H
#define SolutionBuilderMsvcVCProj_H

#include <Core/Heap/Ref.h>
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcProject.h"

class SolutionBuilderMsvcConfiguration;
class SolutionBuilderMsvcTool;
class ProjectItem;

/*! \brief Visual Studio solution project settings. */
class SolutionBuilderMsvcVCProj : public SolutionBuilderMsvcProject
{
	T_RTTI_CLASS(SolutionBuilderMsvcVCProj)

public:
	virtual std::wstring getPlatform() const;

	virtual bool getInformation(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		std::wstring& outProjectPath,
		std::wstring& outProjectFileName,
		std::wstring& outProjectGuid
	) const;

	virtual bool generate(
		GeneratorContext& context,
		Solution* solution,
		Project* project
	) const;

	virtual bool serialize(traktor::Serializer& s);

private:
	std::map< std::wstring, std::wstring > m_staticOptions;
	std::wstring m_platform;
	traktor::Ref< SolutionBuilderMsvcConfiguration > m_configurations[4];	//< One array per target type (executable, console, static library, shared library).
	std::map< std::wstring, traktor::Ref< SolutionBuilderMsvcTool > > m_customTools;

	bool addItem(GeneratorContext& context, Solution* solution, Project* project, ProjectItem* item, traktor::OutputStream& os) const;
};

#endif	// SolutionBuilderMsvcVCProj_H
