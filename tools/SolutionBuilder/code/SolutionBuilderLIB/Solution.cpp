#include <Core/Serialization/Serializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberRef.h>
#include "Solution.h"
#include "Project.h"
#include "ProjectDependency.h"

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"Solution", Solution, traktor::Serializable)

void Solution::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& Solution::getName() const
{
	return m_name;
}

void Solution::setRootPath(const std::wstring& rootPath)
{
	m_rootPath = rootPath;
}

const std::wstring& Solution::getRootPath() const
{
	return m_rootPath;
}

void Solution::addProject(Project* project)
{
	m_projects.push_back(project);
}

void Solution::removeProject(Project* project)
{
	m_projects.remove(project);
	for (traktor::RefList< Project >::iterator i = m_projects.begin(); i != m_projects.end(); ++i)
	{
		traktor::RefList< Dependency >& dependencies = (*i)->getDependencies();
		for (traktor::RefList< Dependency >::iterator j = dependencies.begin(); j != dependencies.end(); )
		{
			if (!traktor::is_a< ProjectDependency >(*j))
			{
				j++;
				continue;
			}

			if (static_cast< ProjectDependency* >(*j)->getProject() == project)
				j = dependencies.erase(j);
			else
				j++;
		}
	}
}

traktor::RefList< Project >& Solution::getProjects()
{
	return m_projects;
}

bool Solution::serialize(traktor::Serializer& s)
{
	s >> traktor::Member< std::wstring >(L"name", m_name);
	s >> traktor::Member< std::wstring >(L"rootPath", m_rootPath);
	s >> traktor::MemberRefList< Project >(L"projects", m_projects);
	return true;
}

