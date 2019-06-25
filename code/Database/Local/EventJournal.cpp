#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Database/IEvent.h"
#include "Database/Local/EventJournal.h"

namespace traktor
{
    namespace db
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.EventJournal", 0, EventJournal, ISerializable)

void EventJournal::addEntry(const Guid& senderGuid, uint64_t sqnr, const IEvent* event)
{
    m_entries.push_back({ senderGuid, sqnr, event });
}

const std::list< EventJournal::Entry >& EventJournal::getEntries() const
{
    return m_entries;
}

void EventJournal::serialize(ISerializer& s)
{
    s >> MemberStlList< Entry, MemberComposite< Entry > >(L"entries", m_entries);
}

bool EventJournal::Entry::serialize(ISerializer& s)
{
    s >> Member< Guid >(L"sender", sender);
    s >> Member< uint64_t >(L"sqnr", sqnr);
    s >> MemberRef< const IEvent >(L"event", event);
    return true;
}

    }
}
