#pragma once

#include "Avalanche/Dictionary.h"
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Thread/Event.h"
#include "Core/Thread/Semaphore.h"
#include "Net/SocketAddressIPv4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AVALANCHE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Thread;

	namespace avalanche
	{

class Client;

class T_DLLCLASS Peer
:	public Object
,	public Dictionary::IListener
{
	T_RTTI_CLASS;

public:
	explicit Peer(const net::SocketAddressIPv4& serverAddress, Dictionary* dictionary);

	virtual ~Peer();

	const net::SocketAddressIPv4& getServerAddress() const;

	// Dictionary::IListener

	virtual void dictionaryPut(const Key& key, const Blob* blob) override final;

private:
	Ref< Client > m_client;
	Dictionary* m_dictionary = nullptr;
	Thread* m_thread = nullptr;
	std::atomic< bool > m_cancel;
	std::atomic< bool > m_finished;
	AlignedVector< Key > m_queue;
	Semaphore m_queueLock;
	Event m_eventQueued;
};

	}
}
