#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/Pipeline/PipelineInstanceCache.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineInstanceCache", PipelineInstanceCache, IPipelineInstanceCache)

PipelineInstanceCache::PipelineInstanceCache(db::Database* database, const std::wstring& cacheDirectory)
:	m_database(database)
,	m_cacheDirectory(cacheDirectory)
{
	// Ensure cache path exist.
	FileSystem::getInstance().makeAllDirectories(m_cacheDirectory);
}

Ref< ISerializable > PipelineInstanceCache::getObjectReadOnly(const Guid& instanceGuid)
{
	DateTime lastModifyDate;

	// First check if this object has already been read during this build.
	std::map< Guid, Ref< ISerializable > >::iterator i = m_readCache.find(instanceGuid);
	if (i != m_readCache.end())
		return i->second;

	// Get instance from database.
	Ref< db::Instance > instance = m_database->getInstance(instanceGuid);
	if (!instance || !instance->getLastModifyDate(lastModifyDate))
		return 0;

	// Generate cached instance filename.
	std::wstring cachedFileName = instanceGuid.format() + L"_" + toString< uint64_t >(lastModifyDate);
	std::wstring cachedPathName = m_cacheDirectory + L"/" + cachedFileName + L".bin";

	// Cached instance is up-to-date; read from cache.
	Ref< IStream > stream = FileSystem::getInstance().open(cachedPathName, File::FmRead);
	if (stream)
	{
		BufferedStream bufferedStream(stream);
		Ref< ISerializable > object = BinarySerializer(&bufferedStream).readObject();
		T_FATAL_ASSERT (object);
		safeClose(stream);
		m_readCache[instanceGuid] = object;
		return object;
	}

	// Either the instance isn't cached yet or not up-to-date; read from database and write a shadow copy in cache.
	Ref< ISerializable > object = instance->getObject();
	m_readCache[instanceGuid] = object;

	if (!object)
		return 0;

	stream = FileSystem::getInstance().open(cachedPathName, File::FmWrite);
	if (stream)
	{
		BinarySerializer(stream).writeObject(object);
		safeClose(stream);
	}

	return object;
}

void PipelineInstanceCache::flush(const Guid& instanceGuid)
{
	std::map< Guid, Ref< ISerializable > >::iterator i = m_readCache.find(instanceGuid);
	if (i != m_readCache.end())
		m_readCache.erase(i);
}

	}
}
