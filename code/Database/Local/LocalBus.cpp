#include <limits>
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
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

class EventJournal : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Entry
	{
		Guid sender;
		uint64_t sqnr;
		Ref< const IEvent > event;

		bool serialize(ISerializer& s)
		{
			s >> Member< Guid >(L"sender", sender);
			s >> Member< uint64_t >(L"sqnr", sqnr);
			s >> MemberRef< const IEvent >(L"event", event);
			return true;
		}
	};

	void addEntry(const Guid& senderGuid, uint64_t sqnr, const IEvent* event)
	{
		m_entries.push_back({ senderGuid, sqnr, event });
	}

	const std::list< Entry >& getEntries() const
	{
		return m_entries;
	}

	virtual void serialize(ISerializer& s) override final
	{
		s >> MemberStlList< Entry, MemberComposite< Entry > >(L"entries", m_entries);
	}

private:
	std::list< Entry > m_entries;
};

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.EventJournal", 0, EventJournal, ISerializable)

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.LocalBus", LocalBus, IProviderBus)

LocalBus::LocalBus(const std::wstring& journalFileName)
:	m_localGuid(Guid::create())
,	m_globalLock(c_guidGlobalLock)
,	m_journalFileName(journalFileName)
{
}

LocalBus::~LocalBus()
{
}

void LocalBus::close()
{
}

bool LocalBus::putEvent(const IEvent* event)
{
	T_ANONYMOUS_VAR(Acquire< Mutex >)(m_globalLock);
	T_ASSERT(event);

	Ref< EventJournal > eventJournal;

	// Read journal.
	{
		Ref< IStream > journalFile = FileSystem::getInstance().open(m_journalFileName, File::FmRead);
		if (journalFile)
		{
			eventJournal = BinarySerializer(journalFile).readObject< EventJournal >();
			journalFile->close();
		}
		if (!eventJournal)
			eventJournal = new EventJournal();
	}

	// Determine sequence number.
	uint64_t sqnr = 0;
	if (!eventJournal->getEntries().empty())
		sqnr = eventJournal->getEntries().back().sqnr + 1;

	// Add entry to journal.
	eventJournal->addEntry(m_localGuid, sqnr, event);

	// Write journal.
	{
		Ref< IStream > journalFile = FileSystem::getInstance().open(m_journalFileName, File::FmWrite);
		if (!journalFile)
		{
			log::error << L"Unable to insert event into database journal; unable to open journal for write." << Endl;
			return false;
		}

		BinarySerializer(journalFile).writeObject(eventJournal);
		journalFile->close();
	}

	return true;
}

bool LocalBus::getEvent(uint64_t& inoutSqnr, Ref< const IEvent >& outEvent, bool& outRemote)
{
	T_ANONYMOUS_VAR(Acquire< Mutex >)(m_globalLock);
	Ref< EventJournal > eventJournal;

	// Read journal.
	{
		Ref< IStream > journalFile = FileSystem::getInstance().open(m_journalFileName, File::FmRead);
		if (journalFile)
		{
			eventJournal = BinarySerializer(journalFile).readObject< EventJournal >();
			journalFile->close();
		}
		if (!eventJournal)
			eventJournal = new EventJournal();
	}

	if (inoutSqnr != std::numeric_limits< uint64_t >::max())
	{
		// Find newer entry than given sequence number.
		for (const auto& entry : eventJournal->getEntries())
		{
			if (entry.sqnr > inoutSqnr)
			{
				inoutSqnr = entry.sqnr;
				outEvent = entry.event;
				outRemote = (bool)(entry.sender != m_localGuid);
				return true;
			}
		}
	}
	else
	{
		// Get last entry.
		const auto& entries = eventJournal->getEntries();
		if (!entries.empty())
		{
			inoutSqnr = entries.back().sqnr;
			outEvent = entries.back().event;
			outRemote = (bool)(entries.back().sender != m_localGuid);
			return true;
		}
	}

	return false;
}

	}
}
