#pragma once

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Thread/ReaderWriterLock.h"

namespace traktor
{

class Guid;
class ISerializable;

	namespace db
	{

class Database;

	}

class ReadOnlyObjectCache : public Object
{
	T_RTTI_CLASS;

public:
	ReadOnlyObjectCache(db::Database* database);

	const ISerializable* get(const Guid& instanceGuid);

private:
	Ref< db::Database > m_database;
	ReaderWriterLock m_readCacheLock;
	SmallMap< Guid, Ref< ISerializable > > m_readCache;
};

}

