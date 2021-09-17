#include "Avalanche/Blob.h"
#include "Avalanche/Dictionary.h"
#include "Avalanche/Client/Client.h"
#include "Avalanche/Server/Peer.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Thread/ThreadPool.h"

namespace traktor
{
	namespace avalanche
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.avalanche.Peer", Peer, Object)

Peer::Peer(const net::SocketAddressIPv4& serverAddress, Dictionary* dictionary)
:	m_client(new Client(serverAddress))
,	m_dictionary(dictionary)
{

	auto fn = [=]()
	{
		// Initially replicate all entries of our dictionary to newly found peer.
		AlignedVector< Key > keys;
		m_dictionary->snapshotKeys(keys);
		for (const auto& key : keys)
		{
			if (m_thread->stopped())
				break;
			if (!m_client->have(key))
			{
				Ref< const Blob > blob = m_dictionary->get(key);
				if (blob)
				{
					log::info << L"Replicating " << key.format() << L" to peer..." << Endl;

					Ref< IStream > readStream = blob->read();
					Ref< IStream > peerStream = m_client->put(key);

					StreamCopy(peerStream, readStream).execute();

					peerStream->close();
					readStream->close();

				}
			}
		}
		log::info << L"Peer up-to-date with our dictionary." << Endl;
	};

	ThreadPool::getInstance().spawn(fn, m_thread);

	m_dictionary->addListener(this);
}

Peer::~Peer()
{
	m_dictionary->removeListener(this);
}

const net::SocketAddressIPv4& Peer::getServerAddress() const
{
	return m_client->getServerAddress();
}

void Peer::dictionaryPut(const Key& key, const Blob* blob)
{
	if (!m_client->have(key))
	{
		log::info << L"Replicating " << key.format() << L" to peer..." << Endl;

		Ref< IStream > readStream = blob->read();
		Ref< IStream > peerStream = m_client->put(key);

		StreamCopy(peerStream, readStream).execute();

		peerStream->close();
		readStream->close();
	}
}

	}
}
