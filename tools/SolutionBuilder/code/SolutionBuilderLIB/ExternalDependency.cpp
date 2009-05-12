#include <Core/Serialization/Serializer.h>
#include <Core/Serialization/MemberRef.h>
#include "ExternalDependency.h"
#include "ProjectDependency.h"
#include "Project.h"
#include "Solution.h"
#include "SolutionLoader.h"

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"ExternalDependency", ExternalDependency, Dependency)

ExternalDependency::ExternalDependency(const std::wstring& solutionFileName, const std::wstring& projectName)
:	m_solutionFileName(solutionFileName)
,	m_projectName(projectName)
{
}

void ExternalDependency::setSolutionFileName(const std::wstring& solutionFileName)
{
	m_solutionFileName = solutionFileName;
}

const std::wstring& ExternalDependency::getSolutionFileName() const
{
	return m_solutionFileName;
}

Solution* ExternalDependency::getSolution() const
{
	return m_solution;
}

Project* ExternalDependency::getProject() const
{
	return m_project;
}

std::wstring ExternalDependency::getName() const
{
	return m_projectName;
}

std::wstring ExternalDependency::getLocation() const
{
	return m_solutionFileName;
}

bool ExternalDependency::resolve(SolutionLoader* solutionLoader)
{
	if (m_solution && m_project)
		return true;

	m_solution = solutionLoader->load(m_solutionFileName);
	if (!m_solution)
		return false;

	const traktor::RefList< Project >& projects = m_solution->getProjects();
	for (traktor::RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		if ((*i)->getName() == m_projectName)
		{
			m_project = *i;
			break;
		}
	}

	if (!m_project)
		return false;

	// Resolve other external dependencies from this dependency.
	traktor::RefList< Dependency > originalDependencies = m_project->getDependencies();
	traktor::RefList< Dependency > resolvedDependencies;

	for (traktor::RefList< Dependency >::const_iterator i = originalDependencies.begin(); i != originalDependencies.end(); ++i)
	{
		if (ProjectDependency* projectDependency = traktor::dynamic_type_cast< ProjectDependency* >(*i))
		{
			traktor::Ref< ExternalDependency > externalDependency = (traktor::gc_new< ExternalDependency >(
				m_solutionFileName,
				projectDependency->getProject()->getName()
			));
			if (externalDependency->resolve(solutionLoader))
				resolvedDependencies.push_back(externalDependency);
			else
				return false;
		}
		if (ExternalDependency* externalDependency = traktor::dynamic_type_cast< ExternalDependency* >(*i))
		{
			if (externalDependency->resolve(solutionLoader))
				resolvedDependencies.push_back(externalDependency);
			else
				return false;
		}
	}

	// Replace dependencies with resolved dependencies.
	m_project->getDependencies().clear();
	m_project->getDependencies().insert(m_project->getDependencies().begin(), resolvedDependencies.begin(), resolvedDependencies.end());
	
	return true;
}

bool ExternalDependency::serialize(traktor::Serializer& s)
{
	s >> traktor::Member< std::wstring >(L"solutionFileName", m_solutionFileName);
	s >> traktor::Member< std::wstring >(L"projectName", m_projectName);
	return true;
}
