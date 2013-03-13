#include <algorithm>
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/System/OS.h"
#include "Core/System/ISharedMemory.h"
#include "Core/Thread/Acquire.h"
#include "Database/IEvent.h"
#include "Database/Local/LocalBus.h"

namespace traktor
{
	namespace db
	{
		namespace
		{

const Guid c_guidGlobalLock(L"{6DC29473-147F-4b3f-8DF5-BBC7EDF79111}");
const uint32_t c_maxPendingEvents = 16;

class EventLog : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Entry
	{
		Guid sender;
		Ref< const IEvent > event;

		bool serialize(ISerializer& s)
		{
			s >> Member< Guid >(L"sender", sender);
			s >> MemberRef< const IEvent >(L"event", event);
			return true;
		}
	};

	void registerPeer(const Guid& localGuid)
	{
		m_pending[localGuid].resize(0);
	}

	void unregisterPeer(const Guid& localGuid)
	{
		std::map< Guid, std::list< Entry > >::iterator i = m_pending.find(localGuid);
		if (i != m_pending.end())
			m_pending.erase(i);
	}

	void addEvent(const Guid& senderGuid, const IEvent* event)
	{
		Entry entry = { senderGuid, event };
		for (std::map< Guid, std::list< Entry > >::iterator i = m_pending.begin(); i != m_pending.end(); ++i)
		{
			std::list< Entry >& entries = i->second;
			while (entries.size() >= c_maxPendingEvents)
				entries.pop_front();
			entries.push_back(entry);
		}
	}

	std::list< Entry >& getEvents(const Guid& localGuid)
	{
		return m_pending[localGuid];
	}

	virtual bool serialize(ISerializer& s)
	{
		return s >> MemberStlMap<
			Guid,
			std::list< Entry >,
			MemberStlPair<
				Guid,
				std::list< Entry >,
				Member< Guid >,
				MemberStlList<
					Entry,
					MemberComposite< Entry >
				>
			>
		> (L"pending", m_pending); 
	}

private:
	std::map< Guid, std::list< Entry > > m_pending;
};

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.EventLog", 0, EventLog, ISerializable)

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.LocalBus", LocalBus, IProviderBus)

LocalBus::LocalBus(const std::wstring& eventFileName)
:	m_localGuid(Guid::create())
,	m_globalLock(c_guidGlobalLock)
{
	T_ANONYMOUS_VAR(Acquire< Mutex >)(m_globalLock);
	Ref< EventLog > eventLog;
	Ref< IStream > eventFile;

	// Create our shared memory object.
	m_shm = OS::getInstance().createSharedMemory(toLower(eventFileName), 512 * 1024);
	T_ASSERT (m_shm);

	if (m_globalLock.existing())
	{
		// Read change log.
		eventFile = m_shm->read();
		if (eventFile)
		{
			eventLog = BinarySerializer(eventFile).readObject< EventLog >();
			eventFile->close();
		}
	}

	if (!eventLog)
	{
		eventLog = new EventLog();
		T_DEBUG(L"Event log created");
	}

	// Register ourself with log.
	eventLog->registerPeer(m_localGuid);

	// Write change log.
	eventFile = m_shm->write();
	if (eventFile)
	{
		BinarySerializer(eventFile).writeObject(eventLog);
		eventFile->close();
	}
	else
		log::error << L"Unable to write back event log into shared memory" << Endl;
}

LocalBus::~LocalBus()
{
	m_shm = 0;
}

void LocalBus::close()
{
	T_ANONYMOUS_VAR(Acquire< Mutex >)(m_globalLock);

	// Read change log.
	Ref< EventLog > eventLog;
	Ref< IStream > eventFile = m_shm->read();
	if (eventFile)
	{
		eventLog = BinarySerializer(eventFile).readObject< EventLog >();
		eventFile->close();
	}
	else
		eventLog = new EventLog();

	// Remove ourself from log.
	eventLog->unregisterPeer(m_localGuid);

	// Write change log.
	eventFile = m_shm->write();
	if (eventFile)
	{
		BinarySerializer(eventFile).writeObject(eventLog);
		eventFile->close();
	}
}

bool LocalBus::putEvent(const IEvent* event)
{
	T_ANONYMOUS_VAR(Acquire< Mutex >)(m_globalLock);
	T_ASSERT (event);

	// Read change log.
	Ref< EventLog > eventLog;
	Ref< IStream > eventFile = m_shm->read();
	if (eventFile)
	{
		eventLog = BinarySerializer(eventFile).readObject< EventLog >();
		eventFile->close();
	}
	else
		eventLog = new EventLog();

	// Add change to log.
	eventLog->addEvent(m_localGuid, event);

	// Write change log.
	eventFile = m_shm->write();
	if (!eventFile)
		return false;

	BinarySerializer(eventFile).writeObject(eventLog);
	eventFile->close();

	return true;
}

bool LocalBus::getEvent(Ref< const IEvent >& outEvent, bool& outRemote)
{
	// Pop from local pending events first.
	if (!m_pendingEvents.empty())
	{
		outEvent = m_pendingEvents.front().event;
		outRemote = m_pendingEvents.front().remote;
		m_pendingEvents.pop_front();
		return true;
	}

	// Read events from global log.
	T_ANONYMOUS_VAR(Acquire< Mutex >)(m_globalLock);

	Ref< IStream > eventFile = m_shm->read();
	if (!eventFile)
		return false;
	
	Ref< EventLog > eventLog = BinarySerializer(eventFile).readObject< EventLog >();
	eventFile->close();

	if (!eventLog)
		return false;

	// Get events pending for our instance.
	std::list< EventLog::Entry >& events = eventLog->getEvents(m_localGuid);
	if (events.empty())
		return false;

	// Move into local pending events.
	for (std::list< EventLog::Entry >::iterator i = events.begin(); i != events.end(); ++i)
	{
		Event event = { i->event, i->sender != m_localGuid };
		m_pendingEvents.push_back(event);
	}
	events.resize(0);

	// Write back event log.
	eventFile = m_shm->write();
	if (eventFile)
	{
		BinarySerializer(eventFile).writeObject(eventLog);
		eventFile->close();
	}

	// Pop from local pending events.
	if (!m_pendingEvents.empty())
	{
		outEvent = m_pendingEvents.front().event;
		outRemote = m_pendingEvents.front().remote;
		m_pendingEvents.pop_front();
		return true;
	}

	return false;
}

	}
}
