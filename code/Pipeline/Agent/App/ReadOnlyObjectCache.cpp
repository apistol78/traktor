/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Database.h"
#include "Pipeline/Agent/App/ReadOnlyObjectCache.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.ReadOnlyObjectCache", ReadOnlyObjectCache, Object)

ReadOnlyObjectCache::ReadOnlyObjectCache(db::Database* database)
:	m_database(database)
{
}

Ref< const ISerializable > ReadOnlyObjectCache::get(const Guid& instanceGuid)
{
	Ref< ISerializable > object;

	// Get object from cache if already acquired.
	{
		m_readCacheLock.acquireReader();
		std::map< Guid, Ref< ISerializable > >::iterator i = m_readCache.find(instanceGuid);
		if (i != m_readCache.end())
			object = i->second;
		m_readCacheLock.releaseReader();
	}

	// If not acquired then read from database.
	if (!object)
	{
		m_readCacheLock.acquireWriter();
		object = m_database->getObjectReadOnly(instanceGuid);
		m_readCache[instanceGuid] = object;
		m_readCacheLock.releaseWriter();
	}

	return object;
}

	}
}
