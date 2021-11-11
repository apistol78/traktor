#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ProjectItem.h"
#include "SolutionBuilder/Configuration.h"
#include "SolutionBuilder/Dependency.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"Project", 1, Project, ISerializable)

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
	for (auto configuration : m_configurations)
	{
		if (configuration->getName() == name)
			return configuration;
	}
	return nullptr;
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
	T_FATAL_ASSERT(s.getVersion() >= 1);

	s >> Member< bool >(L"enable", m_enable);
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< std::wstring >(L"sourcePath", m_sourcePath);
	s >> MemberRefArray< Configuration >(L"configurations", m_configurations);
	s >> MemberRefArray< ProjectItem >(L"items", m_items);
	s >> MemberRefArray< Dependency >(L"dependencies", m_dependencies);
}

	}
}
