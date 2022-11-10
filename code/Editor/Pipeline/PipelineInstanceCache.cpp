#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Reader.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Acquire.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/DeepHash.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/Pipeline/PipelineInstanceCache.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineInstanceCache", PipelineInstanceCache, IPipelineInstanceCache)

PipelineInstanceCache::PipelineInstanceCache(db::Database* database, const std::wstring& cacheDirectory)
:	m_database(database)
,	m_cacheDirectory(cacheDirectory)
{
	// Ensure cache path exist.
	FileSystem::getInstance().makeAllDirectories(m_cacheDirectory);
}

Ref< const ISerializable > PipelineInstanceCache::getObjectReadOnly(const Guid& instanceGuid)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	DateTime lastModifyDate;

	// First check if this object has already been read during this build.
	auto it = m_readCache.find(instanceGuid);
	if (it != m_readCache.end())
	{
		T_FATAL_ASSERT (it->second.object);
#if defined(_DEBUG)
		if (DeepHash(it->second.object).get() != it->second.hash)
			log::warning << L"Instance " << instanceGuid.format() << L" has been modified; expected to be immutable." << Endl;
#endif
		return it->second.object;
	}

	// Get instance from database.
	Ref< db::Instance > instance = m_database->getInstance(instanceGuid);
	if (!instance)
	{
		log::error << L"PipelineInstanceCache::getObjectReadOnly failed; No such instance in database " << instanceGuid.format() << L"." << Endl;
		return nullptr;
	}
	if (!instance->getLastModifyDate(lastModifyDate))
	{
		log::error << L"PipelineInstanceCache::getObjectReadOnly failed; Unable to get \"last modification data\" of instance " << instanceGuid.format() << L"." << Endl;
		return nullptr;
	}

	// Generate cached instance filename.
	std::wstring cachedFileName = instanceGuid.format();
	std::wstring cachedPathName = m_cacheDirectory + L"/" + cachedFileName + L".bin";

	// Read from cache; discard cached item if not matching time stamp.
	Ref< IStream > stream = FileSystem::getInstance().open(cachedPathName, File::FmRead);
	if (stream)
	{
		uint64_t cachedLastModifyDate = 0;
		uint32_t cachedHash = 0;

		BufferedStream bufferedStream(stream);

		Reader(&bufferedStream) >> cachedLastModifyDate;
		Reader(&bufferedStream) >> cachedHash;

		if (cachedLastModifyDate == lastModifyDate)
		{
			Ref< ISerializable > object = BinarySerializer(&bufferedStream).readObject();
			if (object)
			{
				m_readCache[instanceGuid].object = object;
				m_readCache[instanceGuid].hash = cachedHash;
				return object;
			}
		}

		bufferedStream.close();
		stream = nullptr;
	}

	// Either the instance isn't cached yet or not up-to-date; read from database and write a shadow copy in cache.
	Ref< ISerializable > object = instance->getObject();
	if (!object)
	{
		log::error << L"PipelineInstanceCache::getObjectReadOnly failed; Unable to read instance " << instanceGuid.format() << L"." << Endl;
		return nullptr;
	}

	uint32_t hash = DeepHash(object).get();

	m_readCache[instanceGuid].object = object;
	m_readCache[instanceGuid].hash = hash;

	stream = FileSystem::getInstance().open(cachedPathName, File::FmWrite);
	if (stream)
	{
		BufferedStream bufferedStream(stream);

		Writer(&bufferedStream) << uint64_t(lastModifyDate);
		Writer(&bufferedStream) << hash;

		BinarySerializer(&bufferedStream).writeObject(object);

		bufferedStream.close();
		stream = nullptr;
	}

	return object;
}

void PipelineInstanceCache::flush(const Guid& instanceGuid)
{
	// Remove from in-memory map.
	auto it = m_readCache.find(instanceGuid);
	if (it != m_readCache.end())
		m_readCache.erase(it);

	// Generate cached instance filename.
	std::wstring cachedFileName = instanceGuid.format();
	std::wstring cachedPathName = m_cacheDirectory + L"/" + cachedFileName + L".bin";

	// Delete cached instance if exists.
	FileSystem::getInstance().remove(cachedPathName);
}

}
