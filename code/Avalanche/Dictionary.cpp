#include <algorithm>
#include "Avalanche/Blob.h"
#include "Avalanche/Dictionary.h"
#include "Core/Thread/Acquire.h"

namespace traktor
{
	namespace avalanche
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.avalanche.Dictionary", Dictionary, Object)

Ref< Blob > Dictionary::create() const
{
	return new Blob();
}

Ref< const Blob > Dictionary::get(const Key& key) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	auto it = m_blobs.find(key);
	if (it != m_blobs.end())
		return it->second;
	else
		return nullptr;
}

bool Dictionary::put(const Key& key, const Blob* blob)
{
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		m_blobs[key] = blob;
	}
	for (auto listener : m_listeners)
		listener->dictionaryPut(key, blob);
	return true;
}

void Dictionary::snapshotKeys(AlignedVector< Key >& outKeys) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	outKeys.reserve(m_blobs.size());
	for (auto it : m_blobs)
		outKeys.push_back(it.first);
}

void Dictionary::addListener(IListener* listener)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_listeners.push_back(listener);
}

void Dictionary::removeListener(IListener* listener)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	auto it = std::find(m_listeners.begin(), m_listeners.end(), listener);
	m_listeners.erase(it);
}

	}
}
