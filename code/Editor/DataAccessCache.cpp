#include "Core/Io/ChunkMemory.h"
#include "Core/Io/ChunkMemoryStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Acquire.h"
#include "Editor/DataAccessCache.h"

namespace traktor
{
	namespace editor
	{

bool DataAccessCache::create(const Path& cachePath)
{
	m_cachePath = cachePath;
	return FileSystem::getInstance().makeAllDirectories(m_cachePath);
}

void DataAccessCache::destroy() 
{
	// Flush pending writes.
}

Ref< Object > DataAccessCache::readObject(
	uint32_t key,
	const fn_readObject_t& read,
	const fn_writeObject_t& write,
	const fn_createObject_t& create
)
{
	// Get object from object table.
	Ref< ChunkMemory > chunk;
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		auto it = m_objectPool.find(key);
		if (it != m_objectPool.end())
			chunk = it->second;
	}
	if (chunk)
	{
		ChunkMemoryStream cms(chunk, true, false);
		return read(&cms);
	}

	// Try load blob from file.
	Path fileName = m_cachePath.getPathName() + L"/" + str(L"%08x.blob", key);

	Ref< IStream > blobStream = FileSystem::getInstance().open(fileName, File::FmRead);
	if (blobStream)
	{
		Ref< ChunkMemory > blob = new ChunkMemory();

		ChunkMemoryStream cmsw(blob, false, true);
		if (!StreamCopy(&cmsw, blobStream).execute())
			return nullptr;

		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			m_objectPool.insert(std::make_pair(key, blob));
		}

		ChunkMemoryStream cmsr(blob, true, false);
		return read(&cmsr);
	}

	// No cached entry; need to fabricate object.
	Ref< Object > object = create();
	if (!object)
		return nullptr;

	Ref< ChunkMemory > blob = new ChunkMemory();

	ChunkMemoryStream cms(blob, false, true);
	if (!write(object, &cms))
		return nullptr;

	{
		m_objectPool.insert(std::make_pair(key, blob));
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	}

	// Write to physical cache.
	blobStream = FileSystem::getInstance().open(fileName, File::FmWrite);
	if (blobStream)
	{
		ChunkMemoryStream cmsr(blob, true, false);

		if (!StreamCopy(blobStream, &cmsr).execute())
			return nullptr;

		safeClose(blobStream);
	}

	return object;
}

	}
}
