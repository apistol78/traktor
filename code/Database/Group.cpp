/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Database/Group.h"
#include "Database/IGroupEventListener.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Database/Provider/IProviderGroup.h"
#include "Database/Provider/IProviderInstance.h"

namespace traktor::db
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.Group", Group, Object)

std::wstring Group::getName() const
{
	T_ASSERT(m_providerGroup);
	return m_name;
}

std::wstring Group::getPath() const
{
	T_ASSERT(m_providerGroup);

	if (!m_parent)
		return L"";

	std::wstring path = m_parent->getPath();

	if (!path.empty())
		path += L"/" + m_name;
	else
		path = m_name;

	return path;
}

uint32_t Group::getFlags() const
{
	T_ASSERT(m_providerGroup);
	return m_providerGroup->getFlags();
}

bool Group::rename(const std::wstring& name)
{
	T_ASSERT(m_providerGroup);

	if (!m_providerGroup->rename(name))
		return false;

	const std::wstring previousName = m_name;

	m_name = name;

	if (m_groupEventListener)
		m_groupEventListener->groupEventRenamed(this, previousName);

	return true;
}

bool Group::remove()
{
	T_ASSERT(m_providerGroup);

	if (!m_childInstances.empty() || !m_childGroups.empty())
		return false;

	if (!m_providerGroup->remove())
		return false;

	internalDestroy();
	return true;
}

Ref< Group > Group::getGroup(const std::wstring& groupName)
{
	T_ASSERT(m_providerGroup);
	return findChildGroup(this, FindGroupByName(groupName));
}

Ref< Group > Group::createGroup(const std::wstring& groupName)
{
	T_ASSERT(m_providerGroup);

	Ref< Group > group = getGroup(groupName);
	if (group)
		return group;

	Ref< IProviderGroup > providerGroup = m_providerGroup->createGroup(groupName);
	if (!providerGroup)
		return nullptr;

	group = new Group(m_groupEventListener, m_instanceEventListener);
	if (!group->internalCreate(providerGroup, this))
		return nullptr;

	m_childGroups.push_back(group);
	return group;
}

Ref< Instance > Group::getInstance(const std::wstring& instanceName, const TypeInfo* primaryType)
{
	T_ASSERT(m_providerGroup);
	if (!primaryType)
		return findChildInstance(this, FindInstanceByName(instanceName));
	else
	{
		Ref< Instance > instance = findChildInstance(this, FindInstanceByName(instanceName));
		if (!instance || !is_type_of(*primaryType, *instance->getPrimaryType()))
			return nullptr;
		return instance;
	}
}

Ref< Instance > Group::createInstance(const std::wstring& instanceName, uint32_t flags, const Guid* guid)
{
	T_ASSERT(m_providerGroup);

	// Create instance guid, use given if available.
	const Guid instanceGuid = guid ? *guid : Guid::create();
	if (instanceGuid.isNull() || !instanceGuid.isValid())
	{
		log::error << L"Not allowed to create instance with invalid guid." << Endl;
		return nullptr;
	}

	// Remove existing instance if we're about to replace it.
	if (flags & CifReplaceExisting)
	{
		Ref< Instance > instance = getInstance(instanceName);
		if (instance)
		{
			if (!instance->checkout())
				return nullptr;

			if (!(flags & CifKeepExistingGuid))
			{
				if (!instance->setGuid(instanceGuid))
					return nullptr;
			}

			if (!instance->removeAllData())
				return nullptr;

			return instance;
		}
	}

	// Create provider instance.
	Ref< IProviderInstance > providerInstance = m_providerGroup->createInstance(instanceName, instanceGuid);
	if (!providerInstance)
		return nullptr;

	// Create instance object.
	Ref< Instance > instance = new Instance(this);
	if (!instance->internalCreateNew(providerInstance, this))
		return nullptr;

	return instance;
}

Ref< Group > Group::getParent()
{
	T_ASSERT(m_providerGroup);
	return m_parent;
}

bool Group::getChildGroups(RefArray< Group >& outChildGroups)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	outChildGroups = m_childGroups;
	return true;
}

bool Group::getChildInstances(RefArray< Instance >& outChildInstances)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	outChildInstances = m_childInstances;
	return true;
}

Group::Group(IGroupEventListener* groupEventListener, IInstanceEventListener* instanceEventListener)
:	m_groupEventListener(groupEventListener)
,	m_instanceEventListener(instanceEventListener)
,	m_parent(nullptr)
{
}

bool Group::internalCreate(IProviderGroup* providerGroup, Group* parent)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	m_providerGroup = providerGroup;
	m_parent = parent;
	m_name = m_providerGroup->getName();

	m_childGroups.resize(0);
	m_childInstances.resize(0);

	RefArray< IProviderGroup > providerChildGroups;
	RefArray< IProviderInstance > providerChildInstances;
	m_providerGroup->getChildren(providerChildGroups, providerChildInstances);

	m_childGroups.reserve(providerChildGroups.size());
	for (const auto providerChildGroup : providerChildGroups)
	{
		Ref< Group > childGroup = new Group(m_groupEventListener, m_instanceEventListener);
		if (!childGroup->internalCreate(providerChildGroup, this))
			return false;

		m_childGroups.push_back(childGroup);
	}

	m_childInstances.reserve(providerChildInstances.size());
	for (const auto providerChildInstance : providerChildInstances)
	{
		Ref< Instance > childInstance = new Instance(this);
		if (!childInstance->internalCreateExisting(providerChildInstance, this))
			return false;

		m_childInstances.push_back(childInstance);
	}

	return true;
}

void Group::internalDestroy()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	m_instanceEventListener = nullptr;
	m_providerGroup = nullptr;
	m_parent = nullptr;
	m_name = L"";

	for (auto childGroup : m_childGroups)
		childGroup->internalDestroy();

	m_childGroups.resize(0);

	for (auto childInstance : m_childInstances)
		childInstance->internalDestroy();

	m_childInstances.resize(0);
}

bool Group::internalFlushChildInstances()
{
	m_childInstances.resize(0);

	RefArray< IProviderGroup > providerChildGroups;
	RefArray< IProviderInstance > providerChildInstances;
	m_providerGroup->getChildren(providerChildGroups, providerChildInstances);

	m_childInstances.reserve(providerChildInstances.size());
	for (auto providerChildInstance : providerChildInstances)
	{
		Ref< Instance > childInstance = new Instance(this);
		if (!childInstance->internalCreateExisting(providerChildInstance, this))
			return false;

		m_childInstances.push_back(childInstance);
	}

	return true;
}

bool Group::internalAddExtGroup(const std::wstring& groupName)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	RefArray< IProviderGroup > providerChildGroups;
	RefArray< IProviderInstance > providerChildInstances;
	m_providerGroup->getChildren(providerChildGroups, providerChildInstances);

	for (auto providerChildGroup : providerChildGroups)
	{
		if (providerChildGroup->getName() == groupName)
		{
			Ref< Group > childGroup = new Group(m_groupEventListener, m_instanceEventListener);
			if (!childGroup->internalCreate(providerChildGroup, this))
				return false;

			m_childGroups.push_back(childGroup);
			return true;
		}
	}

	return false;
}

bool Group::internalAddExtInstance(const Guid& instanceGuid)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	RefArray< IProviderGroup > providerChildGroups;
	RefArray< IProviderInstance > providerChildInstances;
	m_providerGroup->getChildren(providerChildGroups, providerChildInstances);

	for (auto providerChildInstance : providerChildInstances)
	{
		if (providerChildInstance->getGuid() == instanceGuid)
		{
			Ref< Instance > childInstance = new Instance(this);
			if (!childInstance->internalCreateExisting(providerChildInstance, this))
				return false;

			m_childInstances.push_back(childInstance);
			return true;
		}
	}

	return false;
}

void Group::instanceEventCreated(Instance* instance)
{
	// Add child instance.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		m_childInstances.push_back(instance);
	}
	m_instanceEventListener->instanceEventCreated(instance);
}

void Group::instanceEventRemoved(Instance* instance)
{
	// Remove child instance from list.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		auto i = std::find(m_childInstances.begin(), m_childInstances.end(), instance);
		T_ASSERT(i != m_childInstances.end());
		m_childInstances.erase(i);
	}
	m_instanceEventListener->instanceEventRemoved(instance);
}

void Group::instanceEventGuidChanged(Instance* instance, const Guid& previousGuid)
{
	m_instanceEventListener->instanceEventGuidChanged(instance, previousGuid);
}

void Group::instanceEventRenamed(Instance* instance, const std::wstring& previousName)
{
	m_instanceEventListener->instanceEventRenamed(instance, previousName);
}

void Group::instanceEventCommitted(Instance* instance)
{
	m_instanceEventListener->instanceEventCommitted(instance);
}

}
