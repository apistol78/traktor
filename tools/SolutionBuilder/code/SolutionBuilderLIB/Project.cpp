#include <Core/Serialization/Serializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberRef.h>
#include "Project.h"
#include "ProjectItem.h"
#include "Configuration.h"
#include "Dependency.h"

// Remove this when we have converted all solutions.
#include "ProjectDependency.h"

using namespace traktor;

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"Project", Project, Serializable)

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

RefList< Configuration >& Project::getConfigurations()
{
	return m_configurations;
}

Configuration* Project::getConfiguration(const std::wstring& name) const
{
	for (RefList< Configuration >::const_iterator i = m_configurations.begin(); i != m_configurations.end(); ++i)
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

RefList< ProjectItem >& Project::getItems()
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

RefList< Dependency >& Project::getDependencies()
{
	return m_dependencies;
}

int Project::getVersion() const
{
	return 1;
}

bool Project::serialize(Serializer& s)
{
	if (s.getVersion() >= 1)
		s >> Member< bool >(L"enable", m_enable);

	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< std::wstring >(L"sourcePath", m_sourcePath);
	s >> MemberRefList< Configuration >(L"configurations", m_configurations);
	s >> MemberRefList< ProjectItem >(L"items", m_items);

	// Handle old style dependencies as well.
	// Remove this when we have converted all solutions.
	if (s.getDirection() == Serializer::SdRead)
	{
		RefList< Object > dependencies;
		s >> MemberRefList< Object >(L"dependencies", dependencies);

		m_dependencies.clear();
		for (RefList< Object >::iterator i = dependencies.begin(); i != dependencies.end(); ++i)
		{
			if (is_a< Project >(*i))
				m_dependencies.push_back(gc_new< ProjectDependency >(static_cast< Project* >(*i)));
			else
				m_dependencies.push_back(checked_type_cast< Dependency* >(*i));
		}
	}
	else
		s >> MemberRefList< Dependency >(L"dependencies", m_dependencies);

	return true;
}
