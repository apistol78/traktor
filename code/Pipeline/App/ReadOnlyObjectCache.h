#pragma once

#include <map>
#include "Core/Object.h"
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

	Ref< const ISerializable > get(const Guid& instanceGuid);

private:
	Ref< db::Database > m_database;
	ReaderWriterLock m_readCacheLock;
	std::map< Guid, Ref< ISerializable > > m_readCache;
};

}

