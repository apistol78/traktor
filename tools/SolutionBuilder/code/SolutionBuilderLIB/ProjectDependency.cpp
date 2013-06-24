#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/MemberRef.h>
#include "ProjectDependency.h"
#include "Project.h"

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"ProjectDependency", 1, ProjectDependency, Dependency)

ProjectDependency::ProjectDependency(Project* project)
:	m_project(project)
{
}

void ProjectDependency::setProject(Project* project)
{
	m_project = project;
}

Project* ProjectDependency::getProject() const
{
	return m_project;
}

std::wstring ProjectDependency::getName() const
{
	return m_project->getName();
}

std::wstring ProjectDependency::getLocation() const
{
	return L"";
}

bool ProjectDependency::resolve(SolutionLoader* solutionLoader)
{
	const traktor::RefArray< Dependency >& projectDependencies = m_project->getDependencies();
	for (traktor::RefArray< Dependency >::const_iterator i = projectDependencies.begin(); i != projectDependencies.end(); ++i)
	{
		if (!(*i)->resolve(solutionLoader))
			return false;
	}
	return true;
}

void ProjectDependency::serialize(traktor::ISerializer& s)
{
	if (s.getVersion() >= 1)
		Dependency::serialize(s);

	s >> traktor::MemberRef< Project >(L"project", m_project);
}
