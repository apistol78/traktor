/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "SolutionBuilder/ExternalDependency.h"

#include "Core/Io/FileSystem.h"
#include "Core/Io/Path.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ProjectDependency.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/SolutionLoader.h"

namespace traktor::sb
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sb.ExternalDependency", 3, ExternalDependency, Dependency)

ExternalDependency::ExternalDependency(const std::wstring& solutionFileName, const std::wstring& projectName)
	: m_solutionFileName(solutionFileName)
	, m_projectName(projectName)
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

bool ExternalDependency::resolve(const Path& referringSolutionPath, SolutionLoader* solutionLoader)
{
	if (m_solution && m_project)
		return true;

	const Path refSolutionPath = FileSystem::getInstance().getAbsolutePath(referringSolutionPath);
	const Path externalSolutionFileName(m_solutionFileName);

	Path solutionFileName = Path(refSolutionPath.getPathOnly()) + externalSolutionFileName;
	solutionFileName = solutionFileName.normalized();

	m_solution = solutionLoader->load(solutionFileName.getPathName());
	if (!m_solution)
	{
		log::error << L"Unable to load external solution \"" << solutionFileName.getPathName() << L"\"; corrupt or missing?" << Endl;
		return false;
	}

	for (auto project : m_solution->getProjects())
	{
		if (project->getName() == m_projectName)
		{
			m_project = project;
			break;
		}
	}
	if (!m_project)
	{
		log::error << L"Unable to resolve external dependency \"" << m_projectName << L"\"; no such project in solution \"" << solutionFileName.getPathName() << L"\"" << Endl;
		return false;
	}

	// Resolve other external dependencies from this dependency.
	RefArray< Dependency > resolvedDependencies;
	for (auto dependency : m_project->getDependencies())
	{
		if (auto projectDependency = dynamic_type_cast< ProjectDependency* >(dependency))
		{
			Ref< ExternalDependency > externalDependency = new ExternalDependency(
				solutionFileName.getPathName(),
				projectDependency->getProject()->getName());
			if (externalDependency->resolve(solutionFileName, solutionLoader))
				resolvedDependencies.push_back(externalDependency);
			else
				return false;
		}
		else if (auto externalDependency = dynamic_type_cast< ExternalDependency* >(dependency))
		{
			if (externalDependency->resolve(solutionFileName, solutionLoader))
				resolvedDependencies.push_back(externalDependency);
			else
				return false;
		}
	}

	// Replace dependencies with resolved dependencies.
	m_project->setDependencies(resolvedDependencies);
	return true;
}

void ExternalDependency::serialize(ISerializer& s)
{
	if (s.getVersion() >= 1)
		Dependency::serialize(s);

	s >> Member< std::wstring >(L"solutionFileName", m_solutionFileName);
	s >> Member< std::wstring >(L"projectName", m_projectName);
}

}
