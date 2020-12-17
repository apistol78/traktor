#include "Database/Database.h"
#include "Pipeline/App/ReadOnlyObjectCache.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ReadOnlyObjectCache", ReadOnlyObjectCache, Object)

ReadOnlyObjectCache::ReadOnlyObjectCache(db::Database* database)
:	m_database(database)
{
}

const ISerializable* ReadOnlyObjectCache::get(const Guid& instanceGuid)
{
	Ref< ISerializable > object;

	// Get object from cache if already acquired.
	{
		m_readCacheLock.acquireReader();
		auto it = m_readCache.find(instanceGuid);
		if (it != m_readCache.end())
			object = it->second;
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
