#include <Ui/FileDialog.h>
#include <Ui/MessageBox.h>
#include <Xml/XmlDeserializer.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/Stream.h>
#include "ImportProject.h"
#include "ImportProjectDialog.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ProjectDependency.h"
#include "SolutionBuilderLIB/ExternalDependency.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"ImportProject", ImportProject, CustomTool)

bool ImportProject::execute(ui::Widget* parent, Solution* solution)
{
	bool result = false;

	ui::FileDialog fileDialog;
	fileDialog.create(parent, L"Open other solution", L"SolutionBuilder solutions;*.xms");
	
	Path filePath;
	if (fileDialog.showModal(filePath))
	{
		Ref< Stream > file = FileSystem::getInstance().open(filePath, traktor::File::FmRead);
		if (file)
		{
			Ref< Solution > otherSolution = xml::XmlDeserializer(file).readObject< Solution >();
			file->close();

			ImportProjectDialog importDialog;
			importDialog.create(parent, L"Import project(s)", otherSolution);

			if (importDialog.showModal() == ui::DrOk)
			{
				RefArray< Project > otherProjects;
				importDialog.getSelectedProjects(otherProjects);

				for (RefArray< Project >::iterator i = otherProjects.begin(); i != otherProjects.end(); ++i)
				{
					RefList< Dependency > resolvedDependencies;

					// Find local project for each dependency of the imported projects.
					RefList< Dependency >& dependencies = (*i)->getDependencies();
					for (RefList< Dependency >::iterator j = dependencies.begin(); j != dependencies.end(); ++j)
					{
						if (ProjectDependency* projectDependency = dynamic_type_cast< ProjectDependency* >(*j))
						{
							std::wstring projectName = projectDependency->getName();

							// Find local project with same name.
							const RefList< Project >& projects = solution->getProjects();
							for (RefList< Project >::const_iterator k = projects.begin(); k != projects.end(); ++k)
							{
								if ((*k)->getName() == projectName)
								{
									resolvedDependencies.push_back(gc_new< ProjectDependency >(*k));
									break;
								}
							}
						}
						else if (ExternalDependency* externalDependency = dynamic_type_cast< ExternalDependency* >(*j))
						{
							// Always add external dependencies.
							resolvedDependencies.push_back(externalDependency);
						}
					}

					// Replace dependencies.
					(*i)->getDependencies() = resolvedDependencies;

					// Finally add project to solution.
					solution->addProject(*i);
				}
			}

			importDialog.destroy();
			result = true;
		}
		else
			ui::MessageBox::show(parent, L"Unable to open solution", L"Error", ui::MbIconExclamation | ui::MbOk);
	}

	fileDialog.destroy();
	return result;
}
