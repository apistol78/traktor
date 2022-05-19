#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Misc/SafeDestroy.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ProjectDependency.h"
#include "SolutionBuilder/ExternalDependency.h"
#include "SolutionBuilder/Editor/App/ImportProject.h"
#include "SolutionBuilder/Editor/App/ImportProjectDialog.h"
#include "Ui/MessageBox.h"
#include "Ui/FileDialog.h"
#include "Xml/XmlDeserializer.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.ImportProject", ImportProject, CustomTool)

bool ImportProject::execute(ui::Widget* parent, Solution* solution)
{
	bool result = false;

	ui::FileDialog fileDialog;
	fileDialog.create(parent, type_name(this), L"Open other solution", L"SolutionBuilder solutions;*.xms");

	Path filePath;
	if (fileDialog.showModal(filePath))
	{
		Ref< IStream > file = FileSystem::getInstance().open(filePath, File::FmRead);
		if (file)
		{
			Ref< Solution > otherSolution = xml::XmlDeserializer(file, filePath.getPathName()).readObject< Solution >();
			safeClose(file);

			ImportProjectDialog importDialog;
			importDialog.create(parent, L"Import project(s)", false, otherSolution);

			if (importDialog.showModal() == ui::DrOk)
			{
				RefArray< Project > otherProjects;
				importDialog.getSelectedProjects(otherProjects);

				for (auto otherProject : otherProjects)
				{
					// // Ensure project doesn't already exist in solution.
					auto it = std::find_if(solution->getProjects().begin(), solution->getProjects().end(), [&](const Project* project) {
						return project->getName() == otherProject->getName();
					});
					if (it != solution->getProjects().end())
					{
						ui::MessageBox::show(parent, L"Project " + otherProject->getName() + L" already exists\nin solution.", L"Error", ui::MbIconExclamation | ui::MbOk);
						continue;
					}

					RefArray< Dependency > resolvedDependencies;

					// Find local project for each dependency of the imported projects.
					for (auto dependency : otherProject->getDependencies())
					{
						if (const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(dependency))
						{
							for (auto project : solution->getProjects())
							{
								if (project->getName() == projectDependency->getName())
								{
									resolvedDependencies.push_back(new ProjectDependency(project));
									break;
								}
							}
						}
						else if (ExternalDependency* externalDependency = dynamic_type_cast< ExternalDependency* >(dependency))
						{
							// Always add external dependencies.
							resolvedDependencies.push_back(externalDependency);
						}
					}

					// Replace dependencies.
					otherProject->setDependencies(resolvedDependencies);

					// Finally add project to solution.
					solution->addProject(otherProject);
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

	}
}
