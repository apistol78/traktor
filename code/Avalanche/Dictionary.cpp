#include <algorithm>
#include "Avalanche/BlobFile.h"
#include "Avalanche/BlobMemory.h"
#include "Avalanche/Dictionary.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"

namespace traktor
{
	namespace avalanche
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.avalanche.Dictionary", Dictionary, Object)

bool Dictionary::create(const Path& blobsPath)
{
	if (!blobsPath.empty())
	{
		log::info << L"Loading dictionary..." << Endl;

		if (!FileSystem::getInstance().makeAllDirectories(blobsPath))
			return false;

		RefArray< File > blobFiles;
		FileSystem::getInstance().find(blobsPath.getPathName() + L"/*.blob", blobFiles);

		log::info << L"Loading " << blobFiles.size() << L" blobs..." << Endl;
		for (auto blobFile : blobFiles)
		{
			std::wstring blobFileName = blobFile->getPath().getFileNameNoExtension();

			Key blobKey = Key::parse(blobFileName);
			if (!blobKey.valid())
				continue;

			Ref< BlobFile > blob = new BlobFile(blobFile->getPath(), blobFile->getSize());
			m_blobs[blobKey] = blob;
			m_stats.blobCount++;
			m_stats.memoryUsage += blob->size();
		}
	}

	m_blobsPath = blobsPath;
	return true;
}

Ref< IBlob > Dictionary::create() const
{
	return new BlobMemory();
}

Ref< const IBlob > Dictionary::get(const Key& key) const
{
	Ref< const IBlob > blob;
	{
		T_ANONYMOUS_VAR(ReaderWriterLock::AcquireReader)(m_lockBlobs);
		auto it = m_blobs.find(key);
		if (it == m_blobs.end())
			return nullptr;
		blob = it->second;
	}
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lockListeners);
		for (auto listener : m_listeners)
			listener->dictionaryGet(key);
	}
	return blob;
}

bool Dictionary::put(const Key& key, IBlob* blob)
{
	{
		T_ANONYMOUS_VAR(ReaderWriterLock::AcquireWriter)(m_lockBlobs);

		if (!m_blobsPath.empty())
		{
			const Path blobPath = m_blobsPath.getPathName() + L"/" + key.format() + L".blob";

			// Write blob to physical storage.
			Ref< BlobFile > bf = new BlobFile(blobPath, blob->size());
			if (!StreamCopy(bf->append(), blob->read()).execute())
				return false;
			
			m_blobs[key] = bf;
		}
		else
			m_blobs[key] = blob;

		m_stats.blobCount++;
		m_stats.memoryUsage += blob->size();
	}
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lockListeners);
		for (auto listener : m_listeners)
			listener->dictionaryPut(key, blob);
	}
	return true;
}

bool Dictionary::remove(const Key& key)
{
	{
		T_ANONYMOUS_VAR(ReaderWriterLock::AcquireWriter)(m_lockBlobs);

		auto it = m_blobs.find(key);
		if (it == m_blobs.end())
			return false;

		if (!it->second->remove())
			return false;

		m_blobs.erase(it);
	}
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lockListeners);
		for (auto listener : m_listeners)
			listener->dictionaryRemove(key);
	}
	return true;
}

void Dictionary::snapshotKeys(AlignedVector< Key >& outKeys) const
{
	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireReader)(m_lockBlobs);
	outKeys.reserve(m_blobs.size());
	for (auto it : m_blobs)
		outKeys.push_back(it.first);
}

void Dictionary::addListener(IListener* listener)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lockListeners);
	m_listeners.push_back(listener);
}

void Dictionary::removeListener(IListener* listener)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lockListeners);
	auto it = std::find(m_listeners.begin(), m_listeners.end(), listener);
	m_listeners.erase(it);
}

bool Dictionary::getStats(Stats& outStats) const
{
	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireReader)(m_lockBlobs);
	outStats = m_stats;
	return true;
}

	}
}
