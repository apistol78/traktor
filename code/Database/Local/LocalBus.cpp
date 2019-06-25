#include <limits>
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Thread/Acquire.h"
#include "Database/Local/EventJournal.h"
#include "Database/Local/LocalBus.h"

namespace traktor
{
	namespace db
	{

const Guid c_guidGlobalLock(L"{6DC29473-147F-4b3f-8DF5-BBC7EDF79111}");

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
	T_ANONYMOUS_VAR(Acquire< Mutex >)(m_globalLock);
	m_eventJournal = nullptr;
}

bool LocalBus::putEvent(const IEvent* event)
{
	T_ANONYMOUS_VAR(Acquire< Mutex >)(m_globalLock);
	T_ASSERT(event);

	m_eventJournal = nullptr;

	// Read journal.
	{
		Ref< IStream > journalFile = FileSystem::getInstance().open(m_journalFileName, File::FmRead);
		if (journalFile)
		{
			m_eventJournal = BinarySerializer(journalFile).readObject< EventJournal >();
			journalFile->close();
		}
		if (!m_eventJournal)
			m_eventJournal = new EventJournal();
	}

	// Determine sequence number.
	uint64_t sqnr = 0;
	if (!m_eventJournal->getEntries().empty())
		sqnr = m_eventJournal->getEntries().back().sqnr + 1;

	// Add entry to journal.
	m_eventJournal->addEntry(m_localGuid, sqnr, event);

	// Write journal.
	{
		Ref< IStream > journalFile = FileSystem::getInstance().open(m_journalFileName, File::FmWrite);
		if (!journalFile)
		{
			log::error << L"Unable to insert event into database journal; unable to open journal for write." << Endl;
			return false;
		}

		BinarySerializer(journalFile).writeObject(m_eventJournal);
		journalFile->close();
	}

	return true;
}

bool LocalBus::getEvent(uint64_t& inoutSqnr, Ref< const IEvent >& outEvent, bool& outRemote)
{
	T_ANONYMOUS_VAR(Acquire< Mutex >)(m_globalLock);

	// Check already loaded entries first.
	if (m_eventJournal)
	{
		// Cannot use pre-loaded entries to check last sequence number; need to flush it.
		if (inoutSqnr != std::numeric_limits< uint64_t >::max())
		{
			for (const auto& entry : m_eventJournal->getEntries())
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

		// No entries in loaded journal; need to re-load journal.
		m_eventJournal = nullptr;
	}

	// Read journal.
	{
		Ref< IStream > journalFile = FileSystem::getInstance().open(m_journalFileName, File::FmRead);
		if (journalFile)
		{
			m_eventJournal = BinarySerializer(journalFile).readObject< EventJournal >();
			journalFile->close();
		}
		if (!m_eventJournal)
			m_eventJournal = new EventJournal();
	}

	if (inoutSqnr != std::numeric_limits< uint64_t >::max())
	{
		// Find newer entry than given sequence number.
		for (const auto& entry : m_eventJournal->getEntries())
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
		const auto& entries = m_eventJournal->getEntries();
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
