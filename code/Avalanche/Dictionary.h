#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Io/Path.h"
#include "Core/Misc/Key.h"
#include "Core/Thread/ReaderWriterLock.h"
#include "Core/Thread/Semaphore.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AVALANCHE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace avalanche
	{

class IBlob;

class T_DLLCLASS Dictionary : public Object
{
	T_RTTI_CLASS;

public:
	struct Stats
	{
		uint32_t blobCount = 0;
		uint64_t memoryUsage = 0;
	};

	struct IListener
	{
		virtual void dictionaryGet(const Key& key) = 0;

		virtual void dictionaryPut(const Key& key, const IBlob* blob) = 0;

		virtual void dictionaryRemove(const Key& key) = 0;
	};

	bool create(const Path& blobsPath);

	Ref< IBlob > create() const;

	Ref< IBlob > get(const Key& key) const;

	bool put(const Key& key, IBlob* blob);

	bool remove(const Key& key);

	void snapshotKeys(AlignedVector< Key >& outKeys) const;

	void addListener(IListener* listener);

	void removeListener(IListener* listener);

	bool getStats(Stats& outStats) const;

private:
	mutable ReaderWriterLock m_lockBlobs;
	mutable Semaphore m_lockListeners;
	Path m_blobsPath;
	SmallMap< Key, Ref< IBlob > > m_blobs;
	AlignedVector< IListener* > m_listeners;
	Stats m_stats;
};

	}
}
