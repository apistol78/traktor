#ifndef traktor_db_DbmPutEvent_H
#define traktor_db_DbmPutEvent_H

#include "Database/Remote/IMessage.h"
#include "Database/Types.h"
#include "Core/Guid.h"

namespace traktor
{
	namespace db
	{

/*! \brief Put bus event.
 * \ingroup Database
 */
class DbmPutEvent : public IMessage
{
	T_RTTI_CLASS(DbmPutEvent)

public:
	DbmPutEvent(uint32_t handle = 0, ProviderEvent event = PeInvalid, const Guid& eventId = Guid());

	uint32_t getHandle() const { return m_handle; }

	ProviderEvent getEvent() const { return ProviderEvent(m_event); }

	const Guid& getEventId() const { return m_eventId; }

	virtual bool serialize(Serializer& s);

private:
	uint32_t m_handle;
	int32_t m_event;
	Guid m_eventId;
};

	}
}

#endif	// traktor_db_DbmPutEvent_H
