#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberRefArray.h>
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ProjectItem.h"
#include "SolutionBuilderLIB/Configuration.h"
#include "SolutionBuilderLIB/Dependency.h"

// Remove this when we have converted all solutions.
#include "SolutionBuilderLIB/ProjectDependency.h"

using namespace traktor;

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"Project", 1, Project, ISerializable)

Project::Project()
:	m_enable(true)
{
}

void Project::setEnable(bool enable)
{
	m_enable = enable;
}

bool Project::getEnable() const
{
	return m_enable;
}

void Project::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& Project::getName() const
{
	return m_name;
}

void Project::setSourcePath(const std::wstring& sourcePath)
{
	m_sourcePath = sourcePath;
}

const std::wstring& Project::getSourcePath() const
{
	return m_sourcePath;
}

void Project::addConfiguration(Configuration* configuration)
{
	m_configurations.push_back(configuration);
}

void Project::removeConfiguration(Configuration* configuration)
{
	m_configurations.remove(configuration);
}

const RefArray< Configuration >& Project::getConfigurations() const
{
	return m_configurations;
}

Configuration* Project::getConfiguration(const std::wstring& name) const
{
	for (RefArray< Configuration >::const_iterator i = m_configurations.begin(); i != m_configurations.end(); ++i)
	{
		if ((*i)->getName() == name)
			return *i;
	}
	return 0;
}

void Project::addItem(ProjectItem* item)
{
	m_items.push_back(item);
}

void Project::removeItem(ProjectItem* item)
{
	m_items.remove(item);
}

const RefArray< ProjectItem >& Project::getItems() const
{
	return m_items;
}

void Project::addDependency(Dependency* dependency)
{
	m_dependencies.push_back(dependency);
}

void Project::removeDependency(Dependency* dependency)
{
	m_dependencies.remove(dependency);
}

void Project::setDependencies(const RefArray< Dependency >& dependencies)
{
	m_dependencies = dependencies;
}
	
const RefArray< Dependency >& Project::getDependencies() const
{
	return m_dependencies;
}

void Project::serialize(ISerializer& s)
{
	if (s.getVersion() >= 1)
		s >> Member< bool >(L"enable", m_enable);

	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< std::wstring >(L"sourcePath", m_sourcePath);
	s >> MemberRefArray< Configuration >(L"configurations", m_configurations);
	s >> MemberRefArray< ProjectItem >(L"items", m_items);

	// Handle old style dependencies as well.
	// Remove this when we have converted all solutions.
	if (s.getDirection() == ISerializer::SdRead)
	{
		RefArray< ISerializable > dependencies;
		s >> MemberRefArray< ISerializable >(L"dependencies", dependencies);

		m_dependencies.clear();
		for (RefArray< ISerializable >::iterator i = dependencies.begin(); i != dependencies.end(); ++i)
		{
			if (is_a< Project >(*i))
			{
				Project* project = static_cast< Project* >((*i).ptr());
				m_dependencies.push_back(new ProjectDependency(project));
			}
			else
				m_dependencies.push_back(checked_type_cast< Dependency* >(*i));
		}
	}
	else
		s >> MemberRefArray< Dependency >(L"dependencies", m_dependencies);
}
