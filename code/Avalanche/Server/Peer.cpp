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
	const std::wstring& name,
	Dictionary* dictionary
)
:	m_client(new Client(serverAddress))
,	m_instanceId(instanceId)
,	m_dictionary(dictionary)
,	m_finished(false)
{
	ThreadPool::getInstance().spawn([=]()
		{
			// Initially replicate all entries of our dictionary to newly found peer.
			AlignedVector< Key > keys;
			m_dictionary->snapshotKeys(keys);
			for (const auto& key : keys)
			{
				if (m_thread->stopped())
				{
					m_finished = true;
					return;
				}
				if (!m_client->have(key))
				{
					Ref< const IBlob > blob = m_dictionary->get(key);
					if (blob)
					{
						Ref< IStream > readStream = blob->read();
						Ref< IStream > peerStream = m_client->put(key);
						if (readStream && peerStream)
						{
							log::info << L"Replicating " << key.format() << L" to peer " << name << L"." << Endl;
							StreamCopy(peerStream, readStream).execute();
							peerStream->close();
							readStream->close();
						}
					}
				}
				else
					log::info << L"Skipping " << key.format() << L"; already exists." << Endl;
			}
			if (!keys.empty())
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

void Peer::dictionaryPut(const Key& key, const IBlob* blob)
{
	if (!m_client->have(key))
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_queueLock);
		m_queue.push_back(key);
		m_eventQueued.broadcast();
	}
}

	}
}
