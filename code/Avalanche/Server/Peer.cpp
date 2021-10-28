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
			// Only replicate dictionary from master -> slave.
			if (!m_peerMaster)
			{
				// Get keys registered at peer.
				AlignedVector< Key > remoteKeys;
				m_client->getKeys(remoteKeys);

				// Get our registered keys.
				AlignedVector< Key > localKeys;
				m_dictionary->snapshotKeys(localKeys);

				log::info << L"Synchronize dictionary to peer; " << localKeys.size() << L" local, " << remoteKeys.size() << L" remote keys." << Endl;

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

					log::info << L"Evicting " << remoteKey.format() << L" from peer " << name << L"." << Endl;
					m_client->evict(remoteKey);
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

					Ref< const IBlob > blob = m_dictionary->get(localKey);
					if (!blob)
						continue;

					Ref< IStream > peerStream = m_client->put(localKey);
					if (!peerStream)
					{
						log::error << L"Send of " << localKey.format() << L" failed; unable to create remote blob." << Endl;
						continue;
					}

					Ref< IStream > readStream = blob->read();
					if (readStream)
					{
						log::info << L"Sending " << localKey.format() << L" to peer " << name << L"." << Endl;
						StreamCopy(peerStream, readStream).execute();
						peerStream->close();
						readStream->close();
					}
				}

				log::info << L"Peer " << name << L" up-to-date with our dictionary." << Endl;
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
					{
						log::info << L"Touch " << q.first.format() << L" at peer " << name << L"." << Endl;
						m_client->touch(q.first);
					}
					else if (q.second == c_queueEventPut)
					{
						Ref< const IBlob > blob = m_dictionary->get(q.first);
						if (blob)
						{
							Ref< IStream > readStream = blob->read();
							Ref< IStream > peerStream = m_client->put(q.first);
							if (readStream && peerStream)
							{
								log::info << L"Sending " << q.first.format() << L" to peer " << name << L"." << Endl;
								StreamCopy(peerStream, readStream).execute();
								peerStream->close();
								readStream->close();
							}
						}
					}
					else if (q.second == c_queueEventRemove)
					{
						log::info << L"Evicting " << q.first.format() << L" from peer " << name << L"." << Endl;
						m_client->evict(q.first);
					}
				}
				if (m_queue.empty())
					m_eventQueued.wait(100);
			}

			m_finished = true;
		},
		m_thread
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

	m_eventQueued.broadcast();
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

	m_eventQueued.broadcast();
}

	}
}
