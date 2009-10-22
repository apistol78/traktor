#include <Core/Serialization/Serializer.h>
#include <Core/Serialization/MemberRef.h>
#include "ProjectDependency.h"
#include "Project.h"

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"ProjectDependency", ProjectDependency, Dependency)

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
	const traktor::RefList< Dependency >& projectDependencies = m_project->getDependencies();
	for (traktor::RefList< Dependency >::const_iterator i = projectDependencies.begin(); i != projectDependencies.end(); ++i)
	{
		if (!(*i)->resolve(solutionLoader))
			return false;
	}
	return true;
}

int ProjectDependency::getVersion() const
{
	return 1;
}

bool ProjectDependency::serialize(traktor::Serializer& s)
{
	if (s.getVersion() >= 1)
	{
		if (!Dependency::serialize(s))
			return false;
	}

	return s >> traktor::MemberRef< Project >(L"project", m_project);
}
