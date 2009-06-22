#include <algorithm>
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Database/Provider.h"
#include "Core/Heap/New.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.Group", Group, Object)

Group::Group(IProviderBus* providerBus)
:	m_providerBus(providerBus)
{
}

bool Group::internalCreate(IProviderGroup* providerGroup, Group* parent)
{
	m_providerGroup = providerGroup;
	m_parent = parent;
	m_name = m_providerGroup->getName();

	RefArray< IProviderGroup > providerChildGroups;
	m_providerGroup->getChildGroups(providerChildGroups);

	for (RefArray< IProviderGroup >::iterator i = providerChildGroups.begin(); i != providerChildGroups.end(); ++i)
	{
		Ref< Group > childGroup = gc_new< Group >(m_providerBus);
		if (!childGroup->internalCreate(*i, this))
			return false;

		m_childGroups.push_back(childGroup);
	}

	RefArray< IProviderInstance > providerChildInstances;
	m_providerGroup->getChildInstances(providerChildInstances);

	for (RefArray< IProviderInstance >::iterator i = providerChildInstances.begin(); i != providerChildInstances.end(); ++i)
	{
		Ref< Instance > childInstance = gc_new< Instance >(m_providerBus);
		if (!childInstance->internalCreate(*i, this))
			return false;

		m_childInstances.push_back(childInstance);
	}

	return true;
}

void Group::internalDestroy()
{
	m_providerBus = 0;
	m_providerGroup = 0;
	m_parent = 0;
	m_name = L"";

	for (RefArray< Group >::iterator i = m_childGroups.begin(); i != m_childGroups.end(); ++i)
		(*i)->internalDestroy();

	m_childGroups.resize(0);

	for (RefArray< Instance >::iterator i = m_childInstances.begin(); i != m_childInstances.end(); ++i)
		(*i)->internalDestroy();

	m_childInstances.resize(0);

	Heap::getInstance().invalidateRefs(this);
}

std::wstring Group::getName() const
{
	T_ASSERT (m_providerGroup);
	return m_name;
}

std::wstring Group::getPath() const
{
	T_ASSERT (m_providerGroup);
	
	if (!m_parent)
		return L"";

	std::wstring path = m_parent->getPath();

	if (!path.empty())
		path += L"/" + m_name;
	else
		path = m_name;

	return path;
}

bool Group::rename(const std::wstring& name)
{
	T_ASSERT (m_providerGroup);
	
	if (!m_providerGroup->rename(name))
		return false;

	m_name = name;
	return true;
}

bool Group::remove()
{
	T_ASSERT (m_providerGroup);

	// Remove child instances and groups; each remove will call removeChild... method.
	// If successful we keep iterating until child references are cleared.
	while (!m_childInstances.empty())
	{
		if (!m_childInstances.front()->remove())
			return false;
	}

	while (!m_childGroups.empty())
	{
		if (!m_childGroups.front()->remove())
			return false;
	}

	if (!m_providerGroup->remove())
		return false;

	if (m_parent)
		m_parent->removeChildGroup(this);

	internalDestroy();
	return true;
}

Group* Group::getGroup(const std::wstring& groupName)
{
	T_ASSERT (m_providerGroup);
	return findChildGroup(this, FindGroupByName(groupName));
}

Group* Group::createGroup(const std::wstring& groupName)
{
	T_ASSERT (m_providerGroup);

	Ref< Group > group = getGroup(groupName);
	if (group)
		return group; 

	Ref< IProviderGroup > providerGroup = m_providerGroup->createGroup(groupName);
	if (!providerGroup)
		return 0;

	group = gc_new< Group >(m_providerBus);
	if (!group->internalCreate(providerGroup, this))
		return 0;

	m_childGroups.push_back(group);
	return group;
}

Instance* Group::getInstance(const std::wstring& instanceName)
{
	T_ASSERT (m_providerGroup);
	return findChildInstance(this, FindInstanceByName(instanceName));
}

Instance* Group::createInstance(const std::wstring& instanceName, uint32_t flags, const Guid* guid)
{
	T_ASSERT (m_providerGroup);

	// Create instance guid, use given if available.
	Guid instanceGuid = guid ? *guid : Guid::create();

	// Remove existing instance if we're about to replace it.
	if (flags & CifReplaceExisting)
	{
		Ref< Instance > existingInstance = getInstance(instanceName);
		if (existingInstance)
		{
			if (flags & CifKeepExistingGuid)
				instanceGuid = existingInstance->getGuid();

			if (!existingInstance->checkout())
				return 0;

			if (!existingInstance->remove())
				return 0;

			if (!existingInstance->commit())
				return 0;
		}
	}

	// Create provider instance.
	Ref< IProviderInstance > providerInstance = m_providerGroup->createInstance(instanceName, instanceGuid);
	if (!providerInstance)
		return 0;

	// Create instance object.
	Ref< Instance > instance = gc_new< Instance >(m_providerBus);
	if (!instance->internalCreate(providerInstance, this))
		return 0;

	// @fixme We should add instance when it's finally committed.
	m_childInstances.push_back(instance);
	return instance;
}

Group* Group::getParent()
{
	T_ASSERT (m_providerGroup);
	return m_parent;
}

Group* Group::getFirstChildGroup()
{
	T_ASSERT (m_providerGroup);
	return !m_childGroups.empty() ? m_childGroups[0] : 0;
}

Group* Group::getNextChildGroup(Group* currentGroup)
{
	T_ASSERT (m_providerGroup);
	RefArray< Group >::iterator i = std::find(m_childGroups.begin(), m_childGroups.end(), currentGroup);
	if (i == m_childGroups.end() || ++i == m_childGroups.end())
		return 0;
	return *i;
}

Instance* Group::getFirstChildInstance()
{
	T_ASSERT (m_providerGroup);
	return !m_childInstances.empty() ? m_childInstances[0] : 0;
}

Instance* Group::getNextChildInstance(Instance* currentInstance)
{
	T_ASSERT (m_providerGroup);
	RefArray< Instance >::iterator i = std::find(m_childInstances.begin(), m_childInstances.end(), currentInstance);
	if (i == m_childInstances.end() || ++i == m_childInstances.end())
		return 0;
	return *i;
}

void Group::removeChildInstance(Instance* childInstance)
{
	T_ASSERT (m_providerGroup);
	RefArray< Instance >::iterator i = std::find(m_childInstances.begin(), m_childInstances.end(), childInstance);
	T_ASSERT (i != m_childInstances.end());
	m_childInstances.erase(i);
}

void Group::removeChildGroup(Group* childGroup)
{
	T_ASSERT (m_providerGroup);
	RefArray< Group >::iterator i = std::find(m_childGroups.begin(), m_childGroups.end(), childGroup);
	T_ASSERT (i != m_childGroups.end());
	m_childGroups.erase(i);
}

	}
}
