/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"Solution", 2, Solution, ISerializable)

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

void Solution::addDefinition(const std::wstring& definition)
{
	m_definitions.push_back(definition);
}

void Solution::setDefinitions(const std::vector< std::wstring >& definitions)
{
	m_definitions = definitions;
}

const std::vector< std::wstring >& Solution::getDefinitions() const
{
	return m_definitions;
}

void Solution::addProject(Project* project)
{
	m_projects.push_back(project);
}

void Solution::removeProject(Project* project)
{
	m_projects.remove(project);

	// Remove project from any project dependencies.
	for (RefArray< Project >::iterator i = m_projects.begin(); i != m_projects.end(); ++i)
	{
		RefArray< Dependency > dependencies = (*i)->getDependencies();
		for (RefArray< Dependency >::iterator j = dependencies.begin(); j != dependencies.end(); )
		{
			if (!is_a< ProjectDependency >(*j))
			{
				j++;
				continue;
			}

			if (static_cast< ProjectDependency* >((*j).ptr())->getProject() == project)
				j = dependencies.erase(j);
			else
				j++;
		}
		(*i)->setDependencies(dependencies);
	}

	// Remove project from any aggregation dependencies.
	for (RefArray< Aggregation >::iterator i = m_aggregations.begin(); i != m_aggregations.end(); ++i)
	{
		RefArray< Dependency > dependencies = (*i)->getDependencies();
		for (RefArray< Dependency >::iterator j = dependencies.begin(); j != dependencies.end(); )
		{
			if (!is_a< ProjectDependency >(*j))
			{
				j++;
				continue;
			}

			if (static_cast< ProjectDependency* >((*j).ptr())->getProject() == project)
				j = dependencies.erase(j);
			else
				j++;
		}
		(*i)->setDependencies(dependencies);
	}
}

const RefArray< Project >& Solution::getProjects() const
{
	return m_projects;
}

void Solution::addAggregation(Aggregation* aggregation)
{
	m_aggregations.push_back(aggregation);
}

void Solution::removeAggregation(Aggregation* aggregation)
{
	m_aggregations.remove(aggregation);
}

const RefArray< Aggregation >& Solution::getAggregations() const
{
	return m_aggregations;
}

void Solution::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< std::wstring >(L"rootPath", m_rootPath);

	if (s.getVersion() >= 1)
		s >> MemberStlVector< std::wstring >(L"definitions", m_definitions);

	s >> MemberRefArray< Project >(L"projects", m_projects);

	if (s.getVersion() >= 2)
		s >> MemberRefArray< Aggregation >(L"aggregations", m_aggregations);
}

	}
}
