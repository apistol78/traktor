#pragma once

#include "Avalanche/Dictionary.h"
#include "Core/Guid.h"
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

}

namespace traktor::avalanche
{

class Client;

class T_DLLCLASS Peer
:	public Object
,	public Dictionary::IListener
{
	T_RTTI_CLASS;

public:
	explicit Peer(
		const net::SocketAddressIPv4& serverAddress,
		const Guid& instanceId,
		const bool master,
		const std::wstring& name,
		Dictionary* dictionary
	);

	virtual ~Peer();

	const Guid& getInstanceId() const { return m_instanceId; }

	const net::SocketAddressIPv4& getServerAddress() const;

	// Dictionary::IListener

	virtual void dictionaryGet(const Key& key) override final;

	virtual void dictionaryPut(const Key& key, const IBlob* blob) override final;

	virtual void dictionaryRemove(const Key& key) override final;

private:
	Ref< Client > m_client;
	Guid m_instanceId;
	bool m_peerMaster;
	Dictionary* m_dictionary = nullptr;
	Thread* m_thread = nullptr;
	std::atomic< bool > m_finished;
	AlignedVector< std::pair< Key, uint8_t > > m_queue;
	Semaphore m_queueLock;
	Event m_eventQueued;
};

}
