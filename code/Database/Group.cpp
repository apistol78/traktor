#include <algorithm>
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Database/Provider/IProviderGroup.h"
#include "Database/Provider/IProviderBus.h"

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
		Ref< Group > childGroup = new Group(m_providerBus);
		if (!childGroup->internalCreate(*i, this))
			return false;

		m_childGroups.push_back(childGroup);
	}

	RefArray< IProviderInstance > providerChildInstances;
	m_providerGroup->getChildInstances(providerChildInstances);

	for (RefArray< IProviderInstance >::iterator i = providerChildInstances.begin(); i != providerChildInstances.end(); ++i)
	{
		Ref< Instance > childInstance = new Instance();
		if (!childInstance->internalCreate(m_providerBus, *i, this))
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

Ref< Group > Group::getGroup(const std::wstring& groupName)
{
	T_ASSERT (m_providerGroup);
	return findChildGroup(this, FindGroupByName(groupName));
}

Ref< Group > Group::createGroup(const std::wstring& groupName)
{
	T_ASSERT (m_providerGroup);

	Ref< Group > group = getGroup(groupName);
	if (group)
		return group; 

	Ref< IProviderGroup > providerGroup = m_providerGroup->createGroup(groupName);
	if (!providerGroup)
		return 0;

	group = new Group(m_providerBus);
	if (!group->internalCreate(providerGroup, this))
		return 0;

	m_childGroups.push_back(group);
	return group;
}

Ref< Instance > Group::getInstance(const std::wstring& instanceName, const TypeInfo* primaryType)
{
	T_ASSERT (m_providerGroup);
	if (!primaryType)
		return findChildInstance(this, FindInstanceByName(instanceName));
	else
	{
		Ref< Instance > instance = findChildInstance(this, FindInstanceByName(instanceName));
		if (!instance || !is_type_of(*primaryType, *instance->getPrimaryType()))
			return 0;
		return instance;
	}
}

Ref< Instance > Group::createInstance(const std::wstring& instanceName, uint32_t flags, const Guid* guid)
{
	T_ASSERT (m_providerGroup);
	Ref< Instance > instance;

	// Create instance guid, use given if available.
	Guid instanceGuid = guid ? *guid : Guid::create();

	// Remove existing instance if we're about to replace it.
	if (flags & CifReplaceExisting)
	{
		instance = getInstance(instanceName);
		if (instance)
		{
			if (flags & CifKeepExistingGuid)
				instanceGuid = instance->getGuid();

			if (!instance->checkout())
				return 0;

			if (!instance->remove())
				return 0;

			if (!instance->commit())
				return 0;
		}
	}

	// Create provider instance.
	Ref< IProviderInstance > providerInstance = m_providerGroup->createInstance(instanceName, instanceGuid);
	if (!providerInstance)
		return 0;

	// Create instance object.
	if (!instance)
		instance = new Instance();

	if (!instance->internalCreate(m_providerBus, providerInstance, this))
		return 0;

	// @fixme We should add instance when it's finally committed.
	m_childInstances.push_back(instance);
	return instance;
}

Ref< Group > Group::getParent()
{
	T_ASSERT (m_providerGroup);
	return m_parent;
}

RefArray< Group >::iterator Group::getBeginChildGroup()
{
	return m_childGroups.begin();
}

RefArray< Group >::iterator Group::getEndChildGroup()
{
	return m_childGroups.end();
}

RefArray< Instance >::iterator Group::getBeginChildInstance()
{
	return m_childInstances.begin();
}

RefArray< Instance >::iterator Group::getEndChildInstance()
{
	return m_childInstances.end();
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
