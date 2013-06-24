#include <Core/Log/Log.h>
#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/MemberRef.h>
#include "ExternalDependency.h"
#include "ProjectDependency.h"
#include "Project.h"
#include "Solution.h"
#include "SolutionLoader.h"

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"ExternalDependency", 1, ExternalDependency, Dependency)

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
	{
		traktor::log::error << L"Unable to load external solution \"" << m_solutionFileName << L"\"; corrupt or missing" << traktor::Endl;
		return false;
	}

	const traktor::RefArray< Project >& projects = m_solution->getProjects();
	for (traktor::RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		if ((*i)->getName() == m_projectName)
		{
			m_project = *i;
			break;
		}
	}

	if (!m_project)
	{
		traktor::log::error << L"Unable to resolve external dependency \"" << m_projectName << L"\"; no such project in solution \"" << m_solutionFileName << L"\"" << traktor::Endl;
		return false;
	}

	// Resolve other external dependencies from this dependency.
	traktor::RefArray< Dependency > originalDependencies = m_project->getDependencies();
	traktor::RefArray< Dependency > resolvedDependencies;

	for (traktor::RefArray< Dependency >::const_iterator i = originalDependencies.begin(); i != originalDependencies.end(); ++i)
	{
		if (ProjectDependency* projectDependency = traktor::dynamic_type_cast< ProjectDependency* >(*i))
		{
			traktor::Ref< ExternalDependency > externalDependency = new ExternalDependency(
				m_solutionFileName,
				projectDependency->getProject()->getName()
			);
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
	m_project->setDependencies(resolvedDependencies);
		
	return true;
}

void ExternalDependency::serialize(traktor::ISerializer& s)
{
	if (s.getVersion() >= 1)
		Dependency::serialize(s);

	s >> traktor::Member< std::wstring >(L"solutionFileName", m_solutionFileName);
	s >> traktor::Member< std::wstring >(L"projectName", m_projectName);
}
