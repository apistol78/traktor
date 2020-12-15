#include "Core/Io/ChunkMemory.h"
#include "Core/Io/ChunkMemoryStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Editor/DataAccessCache.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.DataAccessCache", DataAccessCache, Object)

DataAccessCache::~DataAccessCache()
{
	T_FATAL_ASSERT_M(m_writeQueue.empty(), L"Write queue not empty, forgot to destroy instance?");
}

bool DataAccessCache::create(const Path& cachePath)
{
	m_cachePath = cachePath;
	if (!FileSystem::getInstance().makeAllDirectories(m_cachePath))
		return false;

	m_writeThread = ThreadManager::getInstance().create(makeFunctor(this, &DataAccessCache::threadWriter), L"Data access cache");
	if (!m_writeThread)
		return false;

	if (!m_writeThread->start())
	{
		m_writeThread = nullptr;
		return false;
	}

	return true;
}

void DataAccessCache::destroy() 
{
	if (m_writeThread != nullptr)
	{
		// Flush pending writes.
		while (!m_writeQueue.empty())
		{
			m_eventWrite.broadcast();
			ThreadManager::getInstance().getCurrentThread()->yield();
		}

		// Terminate writer thread.
		m_writeThread->stop();
		ThreadManager::getInstance().destroy(m_writeThread);
		m_writeThread = nullptr;
	}
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

	Ref< IStream > blobStream = FileSystem::getInstance().open(fileName, File::FmRead | File::FmMapped);
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

	// Enqueue for writing to physical image.
	m_writeQueue.put({ fileName, blob });
	m_eventWrite.broadcast();

	return object;
}

void DataAccessCache::threadWriter()
{
	Thread* thread = ThreadManager::getInstance().getCurrentThread();
	while (!thread->stopped())
	{
		m_eventWrite.wait(100);

		WriteEntry entry;
		while (m_writeQueue.get(entry))
		{
			Ref< IStream > blobStream = FileSystem::getInstance().open(entry.fileName, File::FmWrite);
			if (blobStream)
			{
				ChunkMemoryStream cmsr(entry.blob, true, false);
				if (!StreamCopy(blobStream, &cmsr).execute())
					log::error << L"Unable to write DAC entry \"" << entry.fileName.getPathName() << L"\" to disc." << Endl;
				safeClose(blobStream);
			}
		}
		entry.blob = nullptr;
	}
}

	}
}
