/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Thread/Acquire.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Database/Events/EvtGroupRenamed.h"
#include "Database/Events/EvtInstanceCommitted.h"
#include "Database/Events/EvtInstanceCreated.h"
#include "Database/Events/EvtInstanceGuidChanged.h"
#include "Database/Events/EvtInstanceRemoved.h"
#include "Database/Events/EvtInstanceRenamed.h"
#include "Database/Provider/IProviderDatabase.h"
#include "Database/Provider/IProviderBus.h"

namespace traktor::db
{
	namespace
	{

void buildInstanceMap(Group* group, SmallMap< Guid, Ref< Instance > >& outInstanceMap)
{
	RefArray< Instance > childInstances;
	group->getChildInstances(childInstances);
	for (const auto childInstance : childInstances)
	{
		outInstanceMap.insert(std::make_pair(
			childInstance->getGuid(),
			childInstance
		));
	}

	RefArray< Group > childGroups;
	group->getChildGroups(childGroups);
	for (const auto childGroup : childGroups)
		buildInstanceMap(childGroup, outInstanceMap);
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.Database", Database, Object)

bool Database::open(IProviderDatabase* providerDatabase)
{
	T_ASSERT(providerDatabase);
	T_ASSERT(!m_providerDatabase);

	m_providerDatabase = providerDatabase;
	m_providerBus = m_providerDatabase->getBus();

	m_lastEntrySqnr = 0;
	if (m_providerBus)
	{
		Ref< const IEvent > event;
		bool remote;

		// Read sequence number of last entry so
		// we know where to start from.
		m_lastEntrySqnr = std::numeric_limits< uint64_t >::max();
		if (!m_providerBus->getEvent(
			m_lastEntrySqnr,
			event,
			remote
		))
		{
			// Will fail to get event if journal is empty; assume first sequence number.
			m_lastEntrySqnr = 0;
		}
	}

	m_rootGroup = new Group(this, this);
	if (!m_rootGroup->internalCreate(m_providerDatabase->getRootGroup(), nullptr))
		return false;

	buildInstanceMap(m_rootGroup, m_instanceMap);
	return true;
}

bool Database::open(const ConnectionString& connectionString)
{
	if (!connectionString.have(L"provider"))
	{
		log::error << L"Unable to open database; provider missing." << Endl;
		return false;
	}

	const TypeInfo* providerType = TypeInfo::find(connectionString.get(L"provider").c_str());
	if (!providerType)
	{
		log::error << L"Unable to open database; no such provider \"" << connectionString.get(L"provider") << L"\"." << Endl;
		return false;
	}

	Ref< IProviderDatabase > providerDatabase = checked_type_cast< IProviderDatabase* >(providerType->createInstance());
	if (!providerDatabase)
	{
		log::error << L"Unable to open database; failed to instanciate provider \"" << connectionString.get(L"provider") << L"\"." << Endl;
		return false;
	}

	if (!providerDatabase->open(connectionString))
		return false;

	return open(providerDatabase);
}

bool Database::create(const ConnectionString& connectionString)
{
	if (!connectionString.have(L"provider"))
	{
		log::error << L"Unable to open database; provider missing." << Endl;
		return false;
	}

	const TypeInfo* providerType = TypeInfo::find(connectionString.get(L"provider").c_str());
	if (!providerType)
	{
		log::error << L"Unable to open database; no such provider \"" << connectionString.get(L"provider") << L"\"." << Endl;
		return false;
	}

	Ref< IProviderDatabase > providerDatabase = checked_type_cast< IProviderDatabase* >(providerType->createInstance());
	if (!providerDatabase)
	{
		log::error << L"Unable to open database; failed to instanciate provider \"" << connectionString.get(L"provider") << L"\"." << Endl;
		return false;
	}

	if (!providerDatabase->create(connectionString))
		return false;

	return open(providerDatabase);
}

void Database::close()
{
	m_instanceMap.clear();

	if (m_rootGroup)
	{
		m_rootGroup->internalDestroy();
		m_rootGroup = nullptr;
	}

	if (m_providerBus)
		m_providerBus = nullptr;

	if (m_providerDatabase)
	{
		m_providerDatabase->close();
		m_providerDatabase = nullptr;
	}
}

Ref< Group > Database::getRootGroup()
{
	T_ASSERT(m_providerDatabase);
	return m_rootGroup;
}

Ref< Group > Database::getGroup(const std::wstring& groupPath)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT(m_providerDatabase);

	Ref< Group > group = m_rootGroup;
	StringSplit< std::wstring > pathElements(groupPath, L"/");
	for (auto i = pathElements.begin(); i != pathElements.end(); ++i)
	{
		if (!(group = findChildGroup(group, FindGroupByName(*i))))
			break;
	}

	return group;
}

Ref< Group > Database::createGroup(const std::wstring& groupPath)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT(m_providerDatabase);

	Ref< Group > group = m_rootGroup;

	StringSplit< std::wstring > groupNames(groupPath, L"/");
	for (auto i = groupNames.begin(); i != groupNames.end(); ++i)
	{
		Ref< Group > childGroup = group->getGroup(*i);
		if (!childGroup)
		{
			childGroup = group->createGroup(*i);
			if (!childGroup)
				return nullptr;
		}
		group = childGroup;
	}

	return group;
}

Ref< Instance > Database::getInstance(const Guid& instanceGuid)
{
	if (instanceGuid.isNull() || !instanceGuid.isValid())
		return nullptr;

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT(m_providerDatabase);

	const auto i = m_instanceMap.find(instanceGuid);
	return i != m_instanceMap.end() ? i->second : nullptr;
}

Ref< Instance > Database::getInstance(const std::wstring& instancePath, const TypeInfo* primaryType)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT(m_providerDatabase);

	std::wstring instanceName = instancePath;
	Ref< Group > group = m_rootGroup;

	const auto i = instanceName.find_last_of(L'/');
	if (i != std::wstring::npos)
	{
		group = getGroup(instanceName.substr(0, i));
		instanceName = instanceName.substr(i + 1);
	}

	if (!group)
		return nullptr;

	return group->getInstance(instanceName, primaryType);
}

Ref< Instance > Database::createInstance(const std::wstring& instancePath, uint32_t flags, const Guid* guid)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT(m_providerDatabase);

	std::wstring instanceName;
	Ref< Group > group;

	const auto i = instancePath.find_last_of(L'/');
	if (i != std::wstring::npos)
	{
		instanceName = instancePath.substr(i + 1);

		std::wstring groupPath = instancePath.substr(0, i);
		if (!groupPath.empty())
			group = createGroup(instancePath.substr(0, i));
		else
			group = m_rootGroup;
	}
	else
	{
		instanceName = instancePath;
		group = m_rootGroup;
	}

	if (instanceName.empty() || !group)
		return nullptr;

	return group->createInstance(instanceName, flags, guid);
}

Ref< ISerializable > Database::getObjectReadOnly(const Guid& guid) const
{
	if (guid.isNull() || !guid.isValid())
		return nullptr;

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT(m_providerDatabase);

	const auto i = m_instanceMap.find(guid);
	if (i == m_instanceMap.end() || !i->second)
		return nullptr;

	return i->second->getObject();
}

bool Database::getEvent(Ref< const IEvent >& outEvent, bool& outRemote)
{
	T_ASSERT(m_providerDatabase);

	if (!m_providerBus || !m_providerBus->getEvent(m_lastEntrySqnr, outEvent, outRemote))
		return false;

	if (outRemote)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		if (dynamic_type_cast< const EvtGroupRenamed* >(outEvent))
		{
			m_instanceMap.clear();
			buildInstanceMap(m_rootGroup, m_instanceMap);
		}

		else if (const EvtInstanceCreated* created = dynamic_type_cast< const EvtInstanceCreated* >(outEvent))
		{
			Ref< Group > group = m_rootGroup;

			StringSplit< std::wstring > pathElements(created->getGroupPath(), L"/");
			for (auto i = pathElements.begin(); group && i != pathElements.end(); ++i)
			{
				Ref< Group > childGroup = findChildGroup(group, FindGroupByName(*i));
				if (childGroup)
					group = childGroup;
				else
				{
					if (!group->internalAddExtGroup(*i))
						log::error << L"Unable to add instance; remotely created group not found." << Endl;
					group = findChildGroup(group, FindGroupByName(*i));
					if (!group)
						log::error << L"Unable to add instance; group \"" << *i << L"\" not found." << Endl;
				}
			}

			if (group)
			{
				if (!group->internalAddExtInstance(created->getInstanceGuid()))
					log::error << L"Unable to add instance; remotely created instance not found." << Endl;
			}

			m_instanceMap.clear();
			buildInstanceMap(m_rootGroup, m_instanceMap);
		}

		else if (const EvtInstanceRemoved* removed = dynamic_type_cast< const EvtInstanceRemoved* >(outEvent))
		{
			auto i = m_instanceMap.find(removed->getInstanceGuid());
			if (i != m_instanceMap.end())
				m_instanceMap.erase(i);
		}

		else if (const EvtInstanceGuidChanged* guidChanged = dynamic_type_cast< const EvtInstanceGuidChanged* >(outEvent))
		{
			auto i = m_instanceMap.find(guidChanged->getInstancePreviousGuid());
			if (i != m_instanceMap.end())
				i->second->internalFlush();

			m_instanceMap.clear();
			buildInstanceMap(m_rootGroup, m_instanceMap);
		}

		else if (const EvtInstanceRenamed* renamed = dynamic_type_cast< const EvtInstanceRenamed* >(outEvent))
		{
			auto i = m_instanceMap.find(renamed->getInstanceGuid());
			if (i != m_instanceMap.end())
			{
				Ref< Group > parent = i->second->getParent();
				if (parent)
					parent->internalFlushChildInstances();
			}

			m_instanceMap.clear();
			buildInstanceMap(m_rootGroup, m_instanceMap);
		}
	}

	return true;
}

void Database::instanceEventCreated(Instance* instance)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	// Insert new cache entry.
	m_instanceMap[instance->getGuid()] = instance;

	// Notify others about new instance.
	if (m_providerBus)
		m_providerBus->putEvent(new EvtInstanceCreated(
			instance->getParent()->getPath(),
			instance->getGuid()
		));
}

void Database::instanceEventRemoved(Instance* instance)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	// Remove previous cached entry.
	const auto i = m_instanceMap.find(instance->getGuid());
	if (i != m_instanceMap.end())
		m_instanceMap.erase(i);

	// Notify others about removed instance.
	if (m_providerBus)
		m_providerBus->putEvent(new EvtInstanceRemoved(instance->getGuid()));
}

void Database::instanceEventGuidChanged(Instance* instance, const Guid& previousGuid)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	// Remove previous cached entry.
	const auto i = m_instanceMap.find(previousGuid);
	if (i != m_instanceMap.end())
		m_instanceMap.erase(i);

	// Insert new cache entry.
	m_instanceMap[instance->getGuid()] = instance;

	// Notify others about instance change.
	if (m_providerBus)
		m_providerBus->putEvent(new EvtInstanceGuidChanged(instance->getGuid(), previousGuid));
}

void Database::instanceEventRenamed(Instance* instance, const std::wstring& previousName)
{
	// Notify others about instance change.
	if (m_providerBus)
		m_providerBus->putEvent(new EvtInstanceRenamed(instance->getGuid(), previousName));
}

void Database::instanceEventCommitted(Instance* instance)
{
	// Notify others about instance committed.
	if (m_providerBus)
		m_providerBus->putEvent(new EvtInstanceCommitted(instance->getGuid()));
}

void Database::groupEventRenamed(Group* group, const std::wstring& previousPath)
{
	// Notify others about group change.
	if (m_providerBus)
		m_providerBus->putEvent(new EvtGroupRenamed(group->getName(), previousPath));
}

}
