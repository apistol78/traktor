#pragma once

#include <list>
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
    namespace db
    {

class IEvent;

/*! Event journal, serializable list of events.
 * \ingroup Database
 */
class EventJournal : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Entry
	{
		Guid sender;
		uint64_t sqnr;
		Ref< const IEvent > event;

		bool serialize(ISerializer& s);
	};

	void addEntry(const Guid& senderGuid, uint64_t sqnr, const IEvent* event);

	const std::list< Entry >& getEntries() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::list< Entry > m_entries;
};

    }
}
