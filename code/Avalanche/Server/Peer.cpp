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
,	m_master(peerMaster)
,	m_dictionary(dictionary)
,	m_finished(false)
{
	ThreadPool::getInstance().spawn([=]()
		{
			// Get keys registered at peer.
			AlignedVector< Key > remoteKeys;
			m_client->getKeys(remoteKeys);

			// Get our registered keys.
			AlignedVector< Key > localKeys;
			m_dictionary->snapshotKeys(localKeys);

			log::info << L"Replicating dictionary to peer; " << localKeys.size() << L" local, " << remoteKeys.size() << L" remote keys." << Endl;

			// Synchronize blobs.
			for (const auto& localKey : localKeys)
			{
				if (m_thread->stopped())
				{
					m_finished = true;
					return;
				}

				if (std::find(remoteKeys.begin(), remoteKeys.end(), localKey) != remoteKeys.end())
					continue;

				if (!m_master)
				{
					// Peer is no master then we send our blob to it.
					Ref< const IBlob > blob = m_dictionary->get(localKey);
					if (!blob)
						continue;

					Ref< IStream > peerStream = m_client->put(localKey);
					if (!peerStream)
					{
						log::error << L"Replication of " << localKey.format() << L" failed; unable to create remote blob." << Endl;
						continue;
					}

					Ref< IStream > readStream = blob->read();
					if (readStream)
					{
						log::info << L"Replicating " << localKey.format() << L" to peer " << name << L"." << Endl;
						StreamCopy(peerStream, readStream).execute();
						peerStream->close();
						readStream->close();
					}
				}
				else
				{
					// Peer is a master so we need to remove local blob.
					log::info << L"Removing " << localKey.format() << L"..." << Endl;
					m_dictionary->remove(localKey);
				}
			}

			log::info << L"Peer " << name << L" up-to-date with our dictionary." << Endl;

			// Process queue of updated blobs.
			while (!m_thread->stopped())
			{
				AlignedVector< Key > queued;
				{
					T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_queueLock);
					m_queue.swap(queued);
				}
				for (size_t i = 0; i < queued.size() && !m_thread->stopped(); ++i)
				{
					const auto& key = queued[i];	
					Ref< const IBlob > blob = m_dictionary->get(key);
					if (blob)
					{
						Ref< IStream > readStream = blob->read();
						Ref< IStream > peerStream = m_client->put(key);
						if (readStream && peerStream)
						{
							log::info << L"[" << (i + 1) << L"/" << queued.size() << L"] Replicating " << key.format() << L" to peer " << name << L"." << Endl;
							StreamCopy(peerStream, readStream).execute();
							peerStream->close();
							readStream->close();
						}
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
}

void Peer::dictionaryPut(const Key& key, const IBlob* blob)
{
	if (m_client->have(key))
		return;

	// Add to event queue.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_queueLock);
		m_queue.push_back(key);
	}

	m_eventQueued.broadcast();
}

void Peer::dictionaryRemove(const Key& key)
{
	if (!m_master)
		m_client->evict(key);
}

	}
}
