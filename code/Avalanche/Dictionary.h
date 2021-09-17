#pragma once

#include "Avalanche/Key.h"
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/SmallMap.h"
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

class Blob;

class T_DLLCLASS Dictionary : public Object
{
	T_RTTI_CLASS;

public:
	struct IListener
	{
		virtual void dictionaryPut(const Key& key, const Blob* blob) = 0;
	};

	Ref< Blob > create() const;

	Ref< const Blob > get(const Key& key) const;

	bool put(const Key& key, const Blob* blob, bool invokeListeners);

	void snapshotKeys(AlignedVector< Key >& outKeys) const;

	void addListener(IListener* listener);

	void removeListener(IListener* listener);

private:
	mutable Semaphore m_lockBlobs;
	mutable Semaphore m_lockListeners;
	SmallMap< Key, Ref< const Blob > > m_blobs;
	AlignedVector< IListener* > m_listeners;
};

	}
}
