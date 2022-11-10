/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStl.h"
#include "SolutionBuilder/Aggregation.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ProjectDependency.h"
#include "SolutionBuilder/Solution.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"Solution", 5, Solution, ISerializable)

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

void Solution::setAggregateOutputPath(const std::wstring& aggregateOutputPath)
{
	m_aggregateOutputPath = aggregateOutputPath;
}

const std::wstring& Solution::getAggregateOutputPath() const
{
	return m_aggregateOutputPath;
}

void Solution::addProject(Project* project)
{
	m_projects.push_back(project);
}

void Solution::removeProject(Project* project)
{
	m_projects.remove(project);

	// Remove project from any project dependencies.
	for (auto project : m_projects)
	{
		RefArray< Dependency > dependencies = project->getDependencies();
		for (auto it = dependencies.begin(); it != dependencies.end(); )
		{
			if (!is_a< ProjectDependency >(*it))
			{
				it++;
				continue;
			}
			if (static_cast< ProjectDependency* >((*it).ptr())->getProject() == project)
				it = dependencies.erase(it);
			else
				it++;
		}
		project->setDependencies(dependencies);
	}
}

void Solution::setProjects(const RefArray< Project >& projects)
{
	m_projects = projects;
}

const RefArray< Project >& Solution::getProjects() const
{
	return m_projects;
}

void Solution::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< std::wstring >(L"rootPath", m_rootPath);

	if (s.getVersion() >= 4)
		s >> Member< std::wstring >(L"aggregateOutputPath", m_aggregateOutputPath);

	if (s.getVersion() >= 1 && s.getVersion() <= 2)
	{
		std::vector< std::wstring > definitions;
		s >> MemberStlVector< std::wstring >(L"definitions", definitions);
	}

	s >> MemberRefArray< Project >(L"projects", m_projects);

	if (s.getVersion() >= 2 && s.getVersion< Solution >() < 5)
	{
		RefArray< Aggregation > aggregations;
		s >> MemberRefArray< Aggregation >(L"aggregations", aggregations);
	}
}

	}
}
