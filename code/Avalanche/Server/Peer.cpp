#include "Avalanche/Dictionary.h"
#include "Avalanche/IBlob.h"
#include "Avalanche/Client/Client.h"
#include "Avalanche/Server/Peer.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/ThreadPool.h"

namespace traktor
{
	namespace avalanche
	{
		namespace
		{

constexpr uint8_t c_queueEventGet = 0x01;
constexpr uint8_t c_queueEventPut = 0x02;
constexpr uint8_t c_queueEventRemove = 0x03;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.avalanche.Peer", Peer, Object)

Peer::Peer(
	const net::SocketAddressIPv4& serverAddress,
	const Guid& instanceId,
	const bool peerMaster,
	const std::wstring& name,
	Dictionary* dictionary
)
:	m_client(new Client(serverAddress))
,	m_instanceId(instanceId)
,	m_peerMaster(peerMaster)
,	m_dictionary(dictionary)
,	m_finished(false)
{
	ThreadPool::getInstance().spawn([=]()
		{
			AlignedVector< Key > touch;
			AlignedVector< Key > evict;

			// Only replicate dictionary from master -> slave.
			if (!m_peerMaster)
			{
				// Get keys registered at peer.
				AlignedVector< Key > remoteKeys;
				m_client->getKeys(remoteKeys);

				// Get our registered keys.
				AlignedVector< Key > localKeys;
				m_dictionary->snapshotKeys(localKeys);

				log::info << L"Synchronizing dictionary with " << name << L"... " << localKeys.size() << L" local, " << remoteKeys.size() << L" remote keys." << Endl;

				// Remove blobs from peer which no longer exist in our dictionary.
				for (const auto& remoteKey : remoteKeys)
				{
					if (m_thread->stopped())
					{
						m_finished = true;
						return;
					}

					if (std::find(localKeys.begin(), localKeys.end(), remoteKey) != localKeys.end())
						continue;

					evict.push_back(remoteKey);
				}
				
				// Update blobs to peer which only exist in our dictionary.
				for (const auto& localKey : localKeys)
				{
					if (m_thread->stopped())
					{
						m_finished = true;
						return;
					}

					if (std::find(remoteKeys.begin(), remoteKeys.end(), localKey) != remoteKeys.end())
						continue;

					Ref< const IBlob > blob = m_dictionary->get(localKey, true);
					if (!blob)
						continue;

					Ref< IStream > peerStream = m_client->put(localKey);
					if (!peerStream)
					{
						log::error << L"Synchronization of " << localKey.format() << L" at " << name << L" failed; unable to create remote blob." << Endl;
						continue;
					}

					Ref< IStream > readStream = blob->read();
					if (readStream)
					{
						log::info << L"Uploading " << localKey.format() << L" to " << name << L"..." << Endl;
						StreamCopy(peerStream, readStream).execute();
						peerStream->close();
						readStream->close();
					}
				}

				log::info << name << L" up-to-date with our dictionary." << Endl;
			}

			// Process queue of updated blobs.
			while (!m_thread->stopped())
			{
				AlignedVector< std::pair< Key, uint8_t > > queued;
				{
					T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_queueLock);
					m_queue.swap(queued);
				}

				for (size_t i = 0; i < queued.size() && !m_thread->stopped(); ++i)
				{
					const auto& q = queued[i];
					if (q.second == c_queueEventGet)
						touch.push_back(q.first);
					else if (q.second == c_queueEventPut)
					{
						Ref< const IBlob > blob = m_dictionary->get(q.first, true);
						if (blob)
						{
							Ref< IStream > readStream = blob->read();
							Ref< IStream > peerStream = m_client->put(q.first);
							if (readStream && peerStream)
							{
								log::info << L"Uploading " << q.first.format() << L" to " << name << L"..." << Endl;
								StreamCopy(peerStream, readStream).execute();
								peerStream->close();
								readStream->close();
							}
						}
					}
					else if (q.second == c_queueEventRemove)
						evict.push_back(q.first);
				}

				if (!touch.empty())
				{
					log::info << L"Touching " << (uint32_t)touch.size() << L" blobs at " << name << L"..." << Endl;
					m_client->touch(touch);
					touch.resize(0);
				}

				if (!evict.empty())
				{
					log::info << L"Evicting " << (uint32_t)evict.size() << L" blobs from " << name << L"..." << Endl;
					m_client->evict(evict);
					evict.resize(0);
				}

				if (m_queue.empty())
					m_eventQueued.wait(500);
			}

			m_finished = true;
		},
		m_thread,
		Thread::Below
	);

	m_dictionary->addListener(this);
}

Peer::~Peer()
{
	if (m_thread && !m_finished)
	{
		m_eventQueued.broadcast();
		ThreadPool::getInstance().stop(m_thread);
		T_FATAL_ASSERT(m_finished);
	}
	m_dictionary->removeListener(this);
}

const net::SocketAddressIPv4& Peer::getServerAddress() const
{
	return m_client->getServerAddress();
}

void Peer::dictionaryGet(const Key& key)
{
	// Only touch to master, since master is in control of eviction.
	if (!m_peerMaster)
		return;

	// Add to event queue.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_queueLock);
		m_queue.push_back({ key, c_queueEventGet });
	}

	// Do not signal queue event since, for now, we try to gather touch in batch.
}

void Peer::dictionaryPut(const Key& key, const IBlob* blob)
{
	if (m_client->have(key))
		return;

	// Add to event queue.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_queueLock);
		m_queue.push_back({ key, c_queueEventPut });
	}

	m_eventQueued.broadcast();
}

void Peer::dictionaryRemove(const Key& key)
{
	// Cannot remove from master, only masters should control eviction.
	if (m_peerMaster)
		return;

	// Add to event queue.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_queueLock);
		m_queue.push_back({ key, c_queueEventRemove });
	}

	// Do not signal queue event since, for now, we try to gather evictions in batch.
}

	}
}
