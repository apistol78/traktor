#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Database/Provider/IProviderDatabase.h"
#include "Database/Provider/IProviderBus.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Core/Thread/Acquire.h"

namespace traktor
{
	namespace db
	{
		namespace
		{

void buildInstanceMap(Group* group, std::map< Guid, Ref< Instance > >& outInstanceMap)
{
	for (RefArray< Instance >::iterator i = group->getBeginChildInstance(); i != group->getEndChildInstance(); ++i)
	{
		Ref< Instance > instance = *i;
		outInstanceMap.insert(std::make_pair(
			instance->getGuid(),
			instance
		));
	}
	for (RefArray< Group >::iterator i = group->getBeginChildGroup(); i != group->getEndChildGroup(); ++i)
		buildInstanceMap(*i, outInstanceMap);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.Database", Database, Object)

bool Database::create(IProviderDatabase* providerDatabase)
{
	T_ASSERT (providerDatabase);
	T_ASSERT (!m_providerDatabase);

	m_providerDatabase = providerDatabase;
	m_providerBus = m_providerDatabase->getBus();

	m_rootGroup = new Group(m_providerBus);
	if (!m_rootGroup->internalCreate(m_providerDatabase->getRootGroup(), 0))
		return false;

	return true;
}

void Database::close()
{
	m_instanceMap.clear();

	if (m_rootGroup)
	{
		m_rootGroup->internalDestroy();
		m_rootGroup = 0;
	}

	if (m_providerBus)
		m_providerBus = 0;

	if (m_providerDatabase)
	{
		m_providerDatabase->close();
		m_providerDatabase = 0;
	}
}

Ref< Group > Database::getRootGroup()
{
	T_ASSERT (m_providerDatabase);
	return m_rootGroup;
}

Ref< Group > Database::getGroup(const std::wstring& groupPath)
{
	T_ASSERT (m_providerDatabase);
	Acquire< Semaphore > scopeLock(m_lock);

	std::vector< std::wstring > pathElements;
	Split< std::wstring >::any(groupPath, L"/", pathElements);

	Ref< Group > group = m_rootGroup;
	for (std::vector< std::wstring >::iterator i = pathElements.begin(); i != pathElements.end(); ++i)
	{
		if (!(group = findChildGroup(group, FindGroupByName(*i))))
			break;
	}

	return group;
}

Ref< Group > Database::createGroup(const std::wstring& groupPath)
{
	T_ASSERT (m_providerDatabase);
	Acquire< Semaphore > scopeLock(m_lock);

	std::vector< std::wstring > groupNames;
	Split< std::wstring >::any(groupPath, L"/", groupNames);

	T_ASSERT (!groupNames.empty());

	Ref< Group > group = m_rootGroup;

	for (std::vector< std::wstring >::iterator i = groupNames.begin(); i != groupNames.end(); ++i)
	{
		Ref< Group > childGroup = group->getGroup(*i);
		if (!childGroup)
		{
			childGroup = group->createGroup(*i);
			if (!childGroup)
				return 0;
		}
		group = childGroup;
	}

	return group;
}

Ref< Instance > Database::getInstance(const Guid& instanceGuid)
{
	T_ASSERT (m_providerDatabase);

	if (instanceGuid.isNull() || !instanceGuid.isValid())
		return 0;

	Acquire< Semaphore > scopeLock(m_lock);
	std::map< Guid, Ref< Instance > >::iterator i = m_instanceMap.find(instanceGuid);
	
	// In case no instance was found or reference has been invalidated we need to rebuild instance map.
	if (i == m_instanceMap.end() || !i->second)
	{
		log::debug << L"Building instance cache" << Endl;

		m_instanceMap.clear();
		buildInstanceMap(m_rootGroup, m_instanceMap);

		if ((i = m_instanceMap.find(instanceGuid)) == m_instanceMap.end())
			return 0;
	}

	return i->second;
}

Ref< Instance > Database::getInstance(const std::wstring& instancePath, const TypeInfo* primaryType)
{
	T_ASSERT (m_providerDatabase);
	Acquire< Semaphore > scopeLock(m_lock);

	std::wstring instanceName = instancePath;
	Ref< Group > group = m_rootGroup;

	std::wstring::size_type i = instanceName.find_last_of(L'/');
	if (i != std::wstring::npos)
	{
		group = getGroup(instanceName.substr(0, i));
		instanceName = instanceName.substr(i + 1);
	}

	if (!group)
		return 0;

	return group->getInstance(instanceName, primaryType);
}

Ref< Instance > Database::createInstance(const std::wstring& instancePath, uint32_t flags, const Guid* guid)
{
	T_ASSERT (m_providerDatabase);
	Acquire< Semaphore > scopeLock(m_lock);

	std::wstring instanceName;
	Ref< Group > group;

	std::wstring::size_type i = instancePath.find_last_of(L'/');
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
		return 0;

	return group->createInstance(instanceName, flags, guid);
}

Ref< ISerializable > Database::getObjectReadOnly(const Guid& guid)
{
	T_ASSERT (m_providerDatabase);
	Acquire< Semaphore > scopeLock(m_lock);

	Ref< Instance > instance = getInstance(guid);
	return instance ? instance->getObject() : 0;
}

bool Database::getEvent(ProviderEvent& outEvent, Guid& outEventId, bool& outRemote)
{
	T_ASSERT (m_providerDatabase);

	if (!m_providerBus)
		return false;

	if (!m_providerBus->getEvent(outEvent, outEventId, outRemote))
		return false;

	// Possibly re-create database as we need to flush instance tree if remote has committed a tree change.
	if (outRemote && (outEvent == PeCommited || outEvent == PeRenamed || outEvent == PeRemoved))
	{
		Acquire< Semaphore > scopeLock(m_lock);

		log::debug << L"Re-creating database tree; remotely modified" << Endl;

		m_instanceMap.clear();

		if (m_rootGroup)
			m_rootGroup->internalDestroy();

		m_rootGroup = new Group(m_providerBus);
		if (!m_rootGroup->internalCreate(m_providerDatabase->getRootGroup(), 0))
			T_FATAL_ERROR;
	}

	return true;
}

	}
}
