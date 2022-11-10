/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <limits>
#include "Core/Io/MemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/System/ISharedMemory.h"
#include "Core/System/OS.h"
#include "Database/IEvent.h"
#include "Database/Local/LocalBus.h"

namespace traktor
{
	namespace db
	{
		namespace
		{

const uint32_t c_maxJournalSize = 16 * 1024 * 1024;

#pragma pack(1)

struct JournalHeader
{
	uint64_t nsqnr;	//!< Next sequence number.
	uint32_t count;	//!< Number of entries in journal.
	uint32_t last;	//!< Offset to last entry.
	uint32_t tail;	//!< Offset to after last entry.
};

struct EntryHeader
{
	uint8_t sender[16];	//!< ID of sender.
	uint64_t sqnr;		//!< Sequence number of entry.
	uint32_t size;		//!< Size of entry event.
};

#pragma pack()

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.LocalBus", LocalBus, IProviderBus)

LocalBus::LocalBus(const std::wstring& journalFileName)
:	m_localGuid(Guid::create())
,	m_journalFileName(journalFileName)
{
	m_shm = OS::getInstance().createSharedMemory(journalFileName, c_maxJournalSize);
	T_FATAL_ASSERT(m_shm != nullptr);
}

LocalBus::~LocalBus()
{
	close();
}

void LocalBus::close()
{
	m_shm = nullptr;
}

bool LocalBus::putEvent(const IEvent* event)
{
	T_FATAL_ASSERT(event);

	uint8_t* wp = (uint8_t*)m_shm->acquireWritePointer();
	if (!wp)
		return false;

	JournalHeader* jh = (JournalHeader*)wp;

	// Prepare tail if first event in journal.
	if (jh->count == 0)
	{
		T_FATAL_ASSERT(jh->tail == 0);
		jh->tail = sizeof(JournalHeader);
	}

	// Write entry header.
	EntryHeader* eh = (EntryHeader*)(wp + jh->tail);
	std::memcpy(eh->sender, m_localGuid, 16);
	eh->sqnr = ++jh->nsqnr;
	eh->size = 0;

	// Write entry event.
	MemoryStream ms(eh + 1, c_maxJournalSize, false, true);
	if (!BinarySerializer(&ms).writeObject(event))
	{
		m_shm->releaseWritePointer();
		return false;
	}

	// Patch entry header with size of event.
	eh->size = (uint32_t)ms.tell();

	// Update journal header.
	jh->count++;
	jh->last = jh->tail;
	jh->tail += sizeof(EntryHeader) + eh->size;

	m_shm->releaseWritePointer();
	return true;
}

bool LocalBus::getEvent(uint64_t& inoutSqnr, Ref< const IEvent >& outEvent, bool& outRemote)
{
	const uint8_t* rp = (const uint8_t*)m_shm->acquireReadPointer();
	if (!rp)
		return false;

	const JournalHeader* jh = (const JournalHeader*)rp;

	if (jh->count == 0)
	{
		m_shm->releaseReadPointer();
		return false;
	}

	if (inoutSqnr != std::numeric_limits< uint64_t >::max())
	{
		// Find newer entry than given sequence number.
		const uint8_t* erp = rp + sizeof(JournalHeader);
		for (uint32_t i = 0; i < jh->count; ++i)
		{
			const EntryHeader* eh = (const EntryHeader*)(erp);
			if (eh->sqnr > inoutSqnr)
			{
				inoutSqnr = eh->sqnr;
				
				MemoryStream ms(eh + 1, c_maxJournalSize);
				outEvent = BinarySerializer(&ms).readObject< const IEvent >();

				outRemote = (bool)(Guid(eh->sender) != m_localGuid);

				m_shm->releaseReadPointer();
				return true;
			}
			erp += sizeof(EntryHeader) + eh->size;
		}
	}
	else
	{
		const EntryHeader* eh = (const EntryHeader*)(rp + jh->last);
		inoutSqnr = eh->sqnr;
				
		MemoryStream ms(eh + 1, c_maxJournalSize);
		outEvent = BinarySerializer(&ms).readObject< const IEvent >();

		outRemote = (bool)(Guid(eh->sender) != m_localGuid);

		m_shm->releaseReadPointer();
		return true;
	}

	m_shm->releaseReadPointer();
	return false;
}

	}
}
